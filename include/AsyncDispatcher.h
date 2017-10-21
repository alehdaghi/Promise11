//
// Created by mahdi on 10/15/17.
//

#ifndef PROMISEC11_DISPATCHER_H
#define PROMISEC11_DISPATCHER_H

#include <thread>
#include <atomic>
#include <condition_variable>
#include <list>
#include "SimpleDispatcher.h"

class AsyncDispatcher : public SimpleDispatcher {
    std::shared_ptr<std::thread> myThread;
    std::list<std::function<void(void)>> queue;

    std::atomic_int         jobs_left;
    std::atomic_bool        bailout;
    std::atomic_bool        finished;
    std::condition_variable job_available_var;
    std::condition_variable wait_var;
    std::mutex              wait_mutex;
    std::mutex              queue_mutex;

    /**
     *  Take the next job in the queue and run it.
     *  Notify the main thread that a job has completed.
     */
    void Task() ;

    std::function<void(void)> next_job();

public:
    AsyncDispatcher();

    AsyncDispatcher(const std::function<void(std::function<void(void)>)> & dispatcher) = delete;

    ~AsyncDispatcher();

    inline unsigned jobsRemaining() ;

    virtual void dispatch( std::function<void(void)> job ) override ;

    void join( bool WaitForAll = true );

    void wait();
};



#endif //PROMISEC11_DISPATCHER_H
