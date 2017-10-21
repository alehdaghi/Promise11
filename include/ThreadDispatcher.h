//
// Created by mahdi on 10/15/17.
//

#ifndef PROMISEC11_THREADDISPATCHER_H
#define PROMISEC11_THREADDISPATCHER_H



#include <stack>
#include "AsyncDispatcher.h"

class ThreadDispatcher {
    static std::stack<std::shared_ptr<SimpleDispatcher>> dispatchers;
public:

    static void pushDispatcher(std::shared_ptr<SimpleDispatcher> dispatcher) {
        dispatchers.push(dispatcher);
    }

    static void popDispatcher() {

        if (dispatchers.size() == 0) {
            throw new std::runtime_error("Current Thread doesn't have Active Dispatchers");
        } else {
            dispatchers.pop();
        }
    }

    static std::shared_ptr<SimpleDispatcher> peekDispatcher() {

        if (dispatchers.size() == 0) {
            throw std::runtime_error("Current Thread doesn't have Active Dispatchers");
        } else {
            return dispatchers.top();
        }
    }

    static void dispatchOnCurrentThread(std::function<void (void)> runnable) {
        peekDispatcher()->dispatch(runnable);
    }
};
std::stack<std::shared_ptr<SimpleDispatcher>> ThreadDispatcher::dispatchers;
#endif //PROMISEC11_THREADDISPATCHER_H
