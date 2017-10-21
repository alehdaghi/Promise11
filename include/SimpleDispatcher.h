//
// Created by mahdi on 10/15/17.
//

#ifndef PROMISEC11_SIMPLEDISPATCHER_H
#define PROMISEC11_SIMPLEDISPATCHER_H


#include <functional>
#include <iostream>

class SimpleDispatcher {
    std::function<void(std::function<void(void)>)> dispatcher;
public:
    SimpleDispatcher() {
        dispatcher = [](std::function<void(void)> runnable){
            runnable();
        };
    }
    SimpleDispatcher(const std::function<void(std::function<void(void)>)> & dispatcher) {
        this->dispatcher = dispatcher;
    }
    virtual void dispatch(std::function<void(void)> job) {
        std::cout<<"SimpleDispatcher dispatch"<<std::endl;
        dispatcher(job);
    }
};


#endif //PROMISEC11_SIMPLEDISPATCHER_H
