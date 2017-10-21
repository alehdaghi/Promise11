//
// Created by mahdi on 10/15/17.
//

#include "AsyncDispatcher.h"


void AsyncDispatcher::Task() {
    while( !bailout ) {
        next_job()();
        --jobs_left;
        wait_var.notify_one();

    }
}


std::function<void(void)> AsyncDispatcher::next_job() {
    std::function<void(void)> res;
    std::unique_lock<std::mutex> job_lock( queue_mutex );

    // Wait for a job if we don't have any.
    job_available_var.wait( job_lock, [this]() ->bool { return queue.size() || bailout; } );

    // Get job from the queue
    if( !bailout ) {
        res = queue.front();
        queue.pop_front();
    }
    else { // If we're bailing out, 'inject' a job into the queue to keep jobs_left accurate.
        res = []{};
        ++jobs_left;
    }
    return res;
}


AsyncDispatcher::AsyncDispatcher()
        : jobs_left( 0 )
        , bailout( false )
        , finished( false )
{
    myThread = std::shared_ptr<std::thread>(new std::thread( [this]{ this->Task(); } ));
}


AsyncDispatcher::~AsyncDispatcher() {
    join();
}


inline unsigned AsyncDispatcher::jobsRemaining() {
    std::lock_guard<std::mutex> guard( queue_mutex );
    return queue.size();
}

void AsyncDispatcher::dispatch( std::function<void(void)> job ) {
    std::lock_guard<std::mutex> guard( queue_mutex );
    queue.emplace_back( job );
    ++jobs_left;
    job_available_var.notify_one();
}

void AsyncDispatcher::join( bool WaitForAll) {
    if( !finished ) {
        if( WaitForAll ) {
            wait();
        }

        bailout = true;
        job_available_var.notify_all();
        if( myThread->joinable() )
            myThread->join();
        finished = true;
    }
}


void AsyncDispatcher::wait() {
    if( jobs_left > 0 ) {
        std::unique_lock<std::mutex> lk( wait_mutex );
        wait_var.wait( lk, [this]{ return this->jobs_left == 0; } );
        lk.unlock();
    }
}