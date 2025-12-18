#ifndef THREADEDMATRIXMULTIPLIER_H
#define THREADEDMATRIXMULTIPLIER_H

#include <pcosynchro/pcohoaremonitor.h>
#include <pcosynchro/pcothread.h>
#include <queue>
#include <vector>

#include "abstractmatrixmultiplier.h"
#include "matrix.h"


///
/// A class that holds the necessary parameters for a thread to do a job.
///
template<class T>
class ComputeParameters {
public:
    const SquareMatrix<T>* A; // pointer to const SquareMatrix<T>
    const SquareMatrix<T>* B;
    SquareMatrix<T>* C;

    int row;
    int col;
    int blockSize;
};


/// As a suggestion, a buffer class that could be used to communicate between
/// the workers and the main thread...
///
/// Here we only wrote two potential methods, but there could be more at the end...
///
template<class T>
class Buffer : public PcoHoareMonitor {
private:

    // BUFFER
    std::queue<ComputeParameters<T>> buffer;
    size_t bufferSize;

    // CONDITIONS
    Condition waitSendJob;
    Condition waitGetJob;
    Condition finishedJobs;

    // SINGLE USE
    Condition ownership;
    bool beingUsed = false;

    // STOPPING
    size_t nbSendWaiting = 0;
    size_t nbGetWaiting = 0;
    size_t nbAcquireWaiting = 0;
    bool stopRequested = false;

public:

    // INFOS
    int nbJobFinished{0}; // Keep this updated

    explicit Buffer(const size_t bufferSize) : bufferSize(bufferSize) { }

    ~Buffer() {
        requestStop();
    }

    ///
    /// \brief Sends a job to the buffer
    /// \param parameters Reference to a ComputeParameters object which holds the necessary parameters to execute a job
    ///
    void sendJob(ComputeParameters<T> parameters) {

        // enter monitor
        monitorIn();

        // check if buffer is full, wait if so
        if (buffer.size() == bufferSize) {
            ++nbSendWaiting;
            wait(waitSendJob);
        }

        // if buffer got stopped, return
        if (stopRequested) {
            monitorOut();
            return;
        }

        // add a job
        buffer.push(parameters);

        // wake up a getter
        if (nbGetWaiting) {
            --nbGetWaiting;
            signal(waitGetJob); // not necessary to put inside but we're already chekcing
        }

        // exit monitor
        monitorOut();
    }

    ///
    /// \brief Requests a job to the buffer
    /// \param parameters Reference to a ComputeParameters object which holds the necessary parameters to execute a job
    /// \return true if a job is available, false otherwise
    ///
    bool getJob(ComputeParameters<T>& parameters) {

        // enter monitor
        monitorIn();

        // check if buffer is empty, if so wait
        if (buffer.empty()) {
            ++nbGetWaiting;
            wait(waitGetJob);
        }

        // if buffer got stopped, return false to say no job got acquired
        if (stopRequested) {
            monitorOut();
            return false;
        }

        // take a job
        parameters = buffer.front();
        buffer.pop();

        // wake up a sender
        if (nbSendWaiting) {
            --nbSendWaiting;
            signal(waitSendJob);
        }

        // exit monitor
        monitorOut();

        // return true to say a job got acquired
        return true;
    }

    ///
    /// @brief Asks the monitor to stop all activities and release threads
    ///
    void requestStop() {

        // enter monitor
        monitorIn();

        // change state of the buffer
        stopRequested = true;

        // wake up send
        for (size_t i = 0 ; i < nbSendWaiting ; ++i)
            signal(waitSendJob);

        // wake up get
        for (size_t i = 0 ; i < nbGetWaiting ; ++i)
            signal(waitGetJob);

        // wake up acquire
        for (size_t i = 0 ; i < nbAcquireWaiting ; ++i)
            signal(ownership);

        // wake up waiter
        signal(finishedJobs);

        // put variables back to 0
        nbSendWaiting = 0;
        nbGetWaiting = 0;
        nbAcquireWaiting = 0;

        // exit monitor
        monitorOut();
    }

    void waitForFinishedJobs(const int goal) {
        monitorIn();
        if (nbJobFinished < goal && !stopRequested)
            wait(finishedJobs);
        nbJobFinished = 0;
        monitorOut();
    }

    int getNbJobFinished() {
        monitorIn();
        const int result = nbJobFinished;
        monitorOut();
        return result;
    }

    void addJobAndCheckIfFinished(const int goal) {

        // enter monitor
        monitorIn();

        // add one job
        ++nbJobFinished;

        // if we finished, signal and reinitialize state
        if (this->nbJobFinished >= goal) {
            signal(finishedJobs);
        }

        // exit monitor
        monitorOut();
    }

    void acquireBuffer() {
        monitorIn();
        if (beingUsed && !stopRequested) {
            ++nbAcquireWaiting;
            wait(ownership);
            --nbAcquireWaiting;
        }
        beingUsed = true;
        monitorOut();
    }

    void releaseBuffer() {
        monitorIn();
        beingUsed = false;
        signal(ownership);
        monitorOut();
    }
};

///
/// A multithreaded multiplicator. multiply() should at least be reentrant.
/// It is up to you to offer very good parallelism.
///
template<class T>
class ThreadedMatrixMultiplier : public AbstractMatrixMultiplier<T>
{
protected:
    int nbThreads;
    int nbBlocksPerRow;
    Buffer<T> buffer;

private:
    std::vector<PcoThread*> threads;

public:
    ///
    /// \brief ThreadedMatrixMultiplier
    /// \param nbThreads Number of threads to start
    /// \param nbBlocksPerRow Default number of blocks per row, for compatibility with SimpleMatrixMultiplier
    ///
    /// The threads shall be started from the constructor
    ///
    explicit ThreadedMatrixMultiplier(const int nbThreads, const int nbBlocksPerRow = 0)
        : nbThreads(nbThreads), nbBlocksPerRow(nbBlocksPerRow), buffer(nbThreads) {

        // launch all threads
        for (int i = 0; i < nbThreads; ++i) {
            threads.push_back(new PcoThread([this]() { multiplySimple(); }));
        }
    }

    ///
    /// In this destructor we should ask for the termination of the computations. They could be aborted without
    /// ending into completion.
    /// All threads have to be
    ///
    ~ThreadedMatrixMultiplier() override {

        buffer.requestStop();

        // ask all threads to stop
        for (int i = 0; i < nbThreads; ++i) {
            threads.at(i)->requestStop();
        }

        // wait for all threads to quit
        for (int i = 0; i < nbThreads; ++i) {
            threads.at(i)->join();
        }
    }

    ///
    /// \brief multiplySimple gets called by the threads to churn on the bits of matrices that are small enough
    /// to get multiplied easily
    ///
    void multiplySimple() {

        // params we receive
        ComputeParameters<T> params;

        // loop while we get jobs and didn't get stopped
        while(buffer.getJob(params) && !PcoThread::thisThread()->stopRequested()) {

            const int rowGeneral = params.row * params.blockSize;
            const int colGeneral = params.col * params.blockSize;

            // loop over rows
            for (int rowC = 0 ; rowC < params.blockSize ; ++rowC) {
                int rowEnd = rowGeneral + rowC;

                // loop over columns
                for (int colC = 0 ; colC < params.blockSize ; ++colC) {
                    int colEnd = colGeneral + colC;

                    // initialize single result
                    T result(0);

                    // add all results for A and B going through them
                    for (int pointer = 0 ; pointer < params.A->size() ; ++pointer) {

                        result += params.A->element(pointer,  colEnd) *
                                  params.B->element(rowEnd,  pointer);
                    }

                    // put result inside C
                    params.C->setElement(rowEnd, colEnd, result);
                }
            }

            // add job and check if we arrived to goal
            buffer.addJobAndCheckIfFinished(nbBlocksPerRow * nbBlocksPerRow);
        }
    }

    ///
    /// \brief multiply
    /// \param A First matrix
    /// \param B Second matrix
    /// \param C Result of A*B
    ///
    /// For compatibility reason with SimpleMatrixMultiplier
    void multiply(const SquareMatrix<T>& A, const SquareMatrix<T>& B, SquareMatrix<T>& C) override {
        multiply(A, B, C, nbBlocksPerRow);
    }

    ///
    /// \brief multiply
    /// \param A First matrix
    /// \param B Second matrix
    /// \param C Result of AxB
    /// \param nbBlocksPerRow Number of blocks per row (or columns)
    /// \throws std::invalid_argument
    ///
    /// Executes the multithreaded computation, by decomposing the matrices into blocks.
    /// nbBlocksPerRow must divide the size of the matrix.
    ///
    void multiply(const SquareMatrix<T>& A, const SquareMatrix<T>& B, SquareMatrix<T>& C, int nbBlocksPerRow ) {

        // sizes must match, otherwise multiplying them is impossible
        if (A.size() != B.size() || A.size() != C.size())
            throw std::invalid_argument("Size mismatch. Matrices must be the same size");

        // number of blocks should be positive
        if (nbBlocksPerRow <= 0)
            throw std::invalid_argument("Number of blocks cannot be negative");

        // nbBlocksPerRow must divide the size of the matrix
        if (A.size() % nbBlocksPerRow)
            throw std::invalid_argument("Cannot divide given matrices in the chosen number of blocks");

        // acquire buffer so we're the one using it
        buffer.acquireBuffer();

        // fix nbBlocksPerRow CAUSE APPARENTLY IT'S ALSO IN CONSTRUCTOR FUCK ME
        this->nbBlocksPerRow = nbBlocksPerRow;

        // we take blockSize based on size and number of rows
        int blockSize = A.size() / nbBlocksPerRow;

        // number of blocks per line
        for (int row = 0; row < nbBlocksPerRow; ++row) {

            // number of blocks per column (same number)
            for (int col = 0; col < nbBlocksPerRow; ++col) {

                // send job to buffer
                buffer.sendJob(ComputeParameters<T>{&A, &B, &C, row, col, blockSize});
            }
        }

        // wait that jobs are all finished
        buffer.waitForFinishedJobs(nbBlocksPerRow * nbBlocksPerRow);

        // release buffer since we finished
        buffer.releaseBuffer();
    }
};




#endif // THREADEDMATRIXMULTIPLIER_H
