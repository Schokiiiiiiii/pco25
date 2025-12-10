#ifndef THREADEDMATRIXMULTIPLIER_H
#define THREADEDMATRIXMULTIPLIER_H

#include <pcosynchro/pcoconditionvariable.h>
#include <pcosynchro/pcohoaremonitor.h>
#include <pcosynchro/pcomutex.h>
#include <pcosynchro/pcosemaphore.h>
#include <pcosynchro/pcothread.h>

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
    /// \param params Reference to a ComputeParameters object which holds the necessary parameters to execute a job
    ///
    void sendJob(ComputeParameters<T> params) {

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
        buffer.push(params);

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
        // TODO
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
    /// \brief multiply
    /// \param A First matrix
    /// \param B Second matrix
    /// \param C Result of AxB
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
    ///
    /// Executes the multithreaded computation, by decomposing the matrices into blocks.
    /// nbBlocksPerRow must divide the size of the matrix.
    ///
    void multiply(const SquareMatrix<T>& A, const SquareMatrix<T>& B, SquareMatrix<T>& C, int nbBlocksPerRow)
    {
        // OK, computation is done correctly, but... Is it really multithreaded?!?
        // TODO : Get rid of the next lines and do something meaningful
        for (int i = 0; i < A.size(); i++) {
            for (int j = 0; j < A.size(); j++) {
                T result = 0.0;
                for (int k = 0; k < A.size(); k++) {
                    result += A.element(k, j) * B.element(i, k);
                }
                C.setElement(i, j, result);
            }
        }
    }

protected:
    int nbThreads;
    int nbBlocksPerRow;
};




#endif // THREADEDMATRIXMULTIPLIER_H
