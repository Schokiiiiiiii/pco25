#ifndef THREADEDMATRIXMULTIPLIER_H
#define THREADEDMATRIXMULTIPLIER_H

#include <pcosynchro/pcohoaremonitor.h>
#include <pcosynchro/pcosemaphore.h>
#include <pcosynchro/pcothread.h>
#include <pcosynchro/pcomutex.h>


#include "abstractmatrixmultiplier.h"
#include "matrix.h"


///
/// A class that holds the necessary parameters for a thread to do a job.
///
template<class T>
class ComputeParameters
{
public:
    const SquareMatrix<T>* A;
    const SquareMatrix<T>* B;
    SquareMatrix<T>* C;

    std::pair<uint, uint> index; // index of position of the block inside the original matrix
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

    // STOPPING
    size_t nbSendWaiting = 0;
    size_t nbGetWaiting = 0;
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
            return false;
        }

        // take a job
        parameters = buffer.front();
        buffer.pop();

        // wake up a sender
        if (nbSendWaiting) {
            --nbGetWaiting;
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

        // put variables back to 0
        nbSendWaiting = 0;
        nbGetWaiting = 0;

        // exit monitor
        monitorOut();
    }
};

///
/// A multi-threaded multiplicator. multiply() should at least be reentrant.
/// It is up to you to offer very good parallelism.
///
template<class T>
class ThreadedMatrixMultiplier : public AbstractMatrixMultiplier<T>
{
protected:
    int nbThreads;
    int nbBlocksPerRow;
    Buffer<T> buffer;
    PcoMutex jobsMutex;

private:
    std::vector<PcoThread*> threads;
    SquareMatrix<T>** results;

public:
    ///
    /// \brief ThreadedMatrixMultiplier
    /// \param nbThreads Number of threads to start
    /// \param nbBlocksPerRow Default number of blocks per row, for compatibility with SimpleMatrixMultiplier
    ///
    /// The threads shall be started from the constructor
    ///
    ThreadedMatrixMultiplier(int nbThreads, int nbBlocksPerRow = 0)
        : nbThreads(nbThreads), nbBlocksPerRow(nbBlocksPerRow), buffer(nbThreads) {

        for (int i = 0; i < nbThreads; ++i) {
            threads.push_back(new PcoThread([this]() { multiplySimple(); }));
        }
        // the following is done this way because we only know the value of nbBlocksPerRow at runtime
        results = new SquareMatrix<T>*[nbBlocksPerRow * nbBlocksPerRow];
    }

    ///
    /// In this destructor we should ask for the termination of the computations. They could be aborted without
    /// ending into completion.
    /// All threads have to be
    ///
    ~ThreadedMatrixMultiplier() {

        for (int i = 0; i < nbThreads; ++i) {
            threads.at(i)->requestStop();
        }
        // in order to avoid undefined behavior, it's best we wait for all the threads to end before we nuke the buffer
        // (because we call getJob)
        for (int i = 0; i < nbThreads; ++i) {
            threads.at(i)->join();
        }
    }

    ///
    /// \brief multiplySimple gets called by the threads to churn on the bits of matrices that are small enough
    /// to get multiplied easily
    ///
    void multiplySimple() {
        ComputeParameters<T> params;
        while(!PcoThread::thisThread()->stopRequested() && buffer.getJob(params)) {
            for (int i = 0; i < params.A->size(); ++i) {
                for (int j = 0; j < params.A->size(); ++j) {
                    T result = 0.0;
                    for (int k = 0; k < params.A->size(); ++k) {
                        result += params.A->element(k, j) * params.B->element(i, k);
                    }
                    params.C->setElement(i, j, result);
                }
            }
            // the way we place the results is not a standard convention (at least to my knowledge)
            // but it just seemed better that way
            results[params.index.first * nbBlocksPerRow + params.index.second] = params.C;

            jobsMutex.lock();
            ++buffer.nbJobFinished;
            if (buffer.nbJobFinished == nbBlocksPerRow * nbBlocksPerRow) buffer.requestStop();
            jobsMutex.unlock();
        }
    }

    ///
    /// \brief multiply
    /// \param A First matrix
    /// \param B Second matrix
    /// \param C Result of A*B
    ///
    /// For compatibility reason with SimpleMatrixMultiplier
    void multiply(const SquareMatrix<T>& A, const SquareMatrix<T>& B, SquareMatrix<T>& C) override
    {
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
    void multiply(const SquareMatrix<T>& A, const SquareMatrix<T>& B, SquareMatrix<T>& C, int nbBlocksPerRow)
    {
        // sizes must match, otherwise multiplying them is impossible
        if (A.getSizeX() * 3 != A.getSizeX() + B.getSizeX() + C.getSizeX())
            throw std::invalid_argument("Size mismatch. Matrices must be the same size");

        // number of blocks should be positive
        if (nbBlocksPerRow < 0) throw std::invalid_argument("Number of blocks cannot be negative");

        // if nbBlocksPerRow == 0, it should be redirected towards multiplySimple
        // (this number is not a very good choice for default but it was decided in the constructor...)
        if (!nbBlocksPerRow) {
            std::pair<uint, uint> position;
            position.first = 0;
            position.second = 0;
            buffer.sendJob(ComputeParameters<T>{&A, &B, &C, position});
        }

        // nbBlocksPerRow must divide the size of the matrix
        if (A.getSizeX() % nbBlocksPerRow)
            throw std::invalid_argument("Cannot divide given matrices in the chosen number of blocks");

        // i know this works, thanks to the check before that
        int blockSize = A.size() / nbBlocksPerRow;

        // number of blocks per line
        for (uint m = 0; m < nbBlocksPerRow; ++m) {       // this represents which block we're looking at
            // number of blocks per column (same number)
            for (uint n = 0; n < nbBlocksPerRow; ++n) {

                SquareMatrix<T> X(blockSize), Y(blockSize), Z(blockSize);
                std::pair<uint, uint> position;

                // copy of the block in X and Y, one element after another
                for (int i = 0; i < blockSize; ++i) {
                    for (int j = 0; j < blockSize; ++j) {
                        X.setElement(i, j, A.element(blockSize * m + i, blockSize * n + j));
                        Y.setElement(i, j, B.element(blockSize * m + i, blockSize * n + j));
                    }
                }

                position.first = m;
                position.second = n;

                buffer.sendJob(ComputeParameters<T>{&X, &Y, &Z, position});
            }
        }

        // if all threads have joined, that means we have finished all the jobs, so results should be full
        for (int i = 0; i < nbThreads; ++i) threads.at(i)->join();

        for (int j = 0; j < nbBlocksPerRow * nbBlocksPerRow; ++j) {
            SquareMatrix<T> temp = *(results[j]); // temp.size() should be equal to blockSize
            for (int x = 0; x < blockSize; ++x) { // lines
                for (int y = 0; y < blockSize; ++y) { // columns (yes i know the names are bad)
                    C.setElement(x + (j / nbBlocksPerRow) * blockSize, y + (j % nbBlocksPerRow) * blockSize, temp.element(x, y));
                }
            }
        }
    }


};




#endif // THREADEDMATRIXMULTIPLIER_H
