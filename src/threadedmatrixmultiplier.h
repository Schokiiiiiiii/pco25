#ifndef THREADEDMATRIXMULTIPLIER_H
#define THREADEDMATRIXMULTIPLIER_H

#include <pcosynchro/pcoconditionvariable.h>
#include <pcosynchro/pcohoaremonitor.h>
#include <pcosynchro/pcomutex.h>
#include <pcosynchro/pcosemaphore.h>
#include <pcosynchro/pcothread.h>

#include <vector>

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

    /* Maybe some parameters */
    // maybe add an index of position inside the big matrix
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
/// It is up to you to offer a very good parallelism.
///
template<class T>
class ThreadedMatrixMultiplier : public AbstractMatrixMultiplier<T>
{
    std::vector<PcoThread*> threads;

public:
    ///
    /// \brief ThreadedMatrixMultiplier
    /// \param nbThreads Number of threads to start
    /// \param nbBlocksPerRow Default number of blocks per row, for compatibility with SimpleMatrixMultiplier
    ///
    /// The threads shall be started from the constructor
    ///
    ThreadedMatrixMultiplier(int nbThreads, int nbBlocksPerRow = 0)
        : nbThreads(nbThreads), nbBlocksPerRow(nbBlocksPerRow)
    {
        for (int i = 0; i < nbThreads; ++i) {
            threads.push_back(new PcoThread(multiplySimple));
        }
    }

    ///
    /// In this destructor we should ask for the termination of the computations. They could be aborted without
    /// ending into completion.
    /// All threads have to be
    ///
    ~ThreadedMatrixMultiplier()
    {
        // TODO
    }

    ///
    /// \brief multiplySimple gets called by the threads to churn on the bits of matrices that are small enough
    /// to get multiplied easily
    ///
    void multiplySimple() {
        ComputeParameters<T>() params;
        while(Buffer::getJob(params)) {
            for (int i = 0; i < A.size(); i++) {
                for (int j = 0; j < A.size(); j++) {
                    T result = 0.0;
                    for (int k = 0; k < A.size(); k++) {
                        result += A.element(k, j) * B.element(i, k);
                    }
                    C.setElement(i, j, result);
                }
            }
            // if the thread made it here, normally, its job is done
            return;
        }
    }

    ///
    /// \brief multiply
    /// \param A First matrix
    /// \param B Second matrix
    /// \param C Result of AxB
    ///
    /// For compatibility reason with SimpleMatrixMultiplier
    void multiply(const SquareMatrix<T>& A, const SquareMatrix<T>& B, SquareMatrix<T>& C) override
    {
        // sizes must match, otherwise multiplying them is impossible
        if (A.getSizeX() * 3 != A.getSizeX() + B.getSizeX() + C.getSizeX()) {
            std::cerr << "Can't multiply given matrices, size mismatch\n";
            return;
        }
        multiply(A, B, C, nbBlocksPerRow);
    }

    ///
    /// \brief multiply
    /// \param A First matrix
    /// \param B Second matrix
    /// \param C Result of AxB
    /// \param nbBlocksPerRow Number of blocks per row (or columns)
    ///
    /// Executes the multithreaded computation, by decomposing the matrices into blocks.
    /// nbBlocksPerRow must divide the size of the matrix.
    ///
    void multiply(const SquareMatrix<T>& A, const SquareMatrix<T>& B, SquareMatrix<T>& C, int nbBlocksPerRow)
    {
        // TODO : Watch out if nbBlocksPerRow == 0, it should be redirected towards multiplySimple perhaps?
        // (this number is not a very good choice for default but it was decided in the constructor...)

        // nbBlocksPerRow must divide the size of the matrix
        if (A.getSizeX() % nbBlocksPerRow) {
            std::cerr << "Can't divide given matrices in " << nbBlocksPerRow << " blocks\n";
            return;
        }

        // i know this works, thanks to the check before that
        int blockSize = A.size() / nbBlocksPerRow;

        // number of blocks per line
        for (int m = 0; m < nbBlocksPerRow; ++m) {       // this represents which block we're looking at
            // number of blocks per column (same number)
            for (int n = 0; n < nbBlocksPerRow; ++n) {

                const SquareMatrix<T> X(blockSize), Y(blockSize);
                SquareMatrix Z(blockSize);

                // copy of the block in X and Y, one element after another
                for (int i = 0; i < blockSize; ++i) {
                    for (int j = 0; j < blockSize; ++j) {
                        X.setElement(i, j, A.element(blockSize * m + i, blockSize * n + j));
                        Y.setElement(i, j, B.element(blockSize * m + i, blockSize * n + j));
                    }
                }

                Buffer::sendJob(new ComputeParameters<T>(X, Y, Z));
            }
        }
    }

protected:
    int nbThreads;
    int nbBlocksPerRow;
};




#endif // THREADEDMATRIXMULTIPLIER_H
