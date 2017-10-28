//
// Created by mahdi on 10/15/17.
//

#ifndef PROMISEC11_PROMISE_HPP_H
#define PROMISEC11_PROMISE_HPP_H


#include <exception>
#include <functional>
#include <list>
#include <tr1/memory>
#include <bits/shared_ptr.h>
#include <vector>

#include "AsyncDispatcher.h"
#include "PromiseResolver.h"
#include "ThreadDispatcher.h"


template<typename T>
class Promise {
    std::recursive_mutex                       mutex;

public:
    using Result_ptr = std::shared_ptr<T>;
    using Exception = std::shared_ptr<std::exception>;
    using PromiseFunc = std::function<void (PromiseResolver<T>)>;

    using shared_ptr = std::shared_ptr<Promise<T>>;
    using weak_ptr = std::weak_ptr<Promise<T>>;

    using OnResult = std::function<void(const T&)>;
    using OnError  = std::function<void(Exception)>;
    using OnAfter  = std::function<void(Result_ptr, Exception)>;

    template<typename R>
    using Function = std::function<R(const T&)>;

    template<typename R>
    using FunctionPromise = std::function<std::shared_ptr<Promise<R>>(const T&)>;


    Promise(const Promise<T>& promise) = delete;
    Promise(const Promise<T>&& promise) = delete;
    Promise<T> operator = (const Promise<T>& promise) = delete;

private:
    friend class PromiseResolver<T>;
    weak_ptr self;
    std::shared_ptr<T> result = nullptr;
    Exception exception = nullptr;
    bool isFinished = false;
    std::vector<OnResult> callbacks = {};
    std::vector<OnError> failures = {};
    std::shared_ptr<SimpleDispatcher> dispatcher;


    /**
    * Successful constructor of promise
    *
    * @param value value
    */

    Promise(const T& value) {
        dispatcher = ThreadDispatcher::peekDispatcher();
        result = std::make_shared<T>(value);
        exception = nullptr;
        isFinished = true;
    }

    /**
     * Exception constructor of promise
     *
     * @param e exception
     */

    Promise(Exception e) {
        dispatcher = ThreadDispatcher::peekDispatcher();
        result = nullptr;
        exception = e;
        isFinished = true;
    }

private:
    // list of shared_ptr to hold a reference to this and prevent to call deconstructor if we have callback to do or
    // are waiting until result become ready
    shared_ptr result_lock;
    shared_ptr callback_lock;


    std::string name;


    Promise() {
        dispatcher = ThreadDispatcher::peekDispatcher();
    }

    void setExecuter(PromiseFunc executor, shared_ptr self) {
        result_lock = self;
        PromiseResolver<T> pr(self);
        executor(pr);
    }



public:

    static shared_ptr createPromise(PromiseFunc executor, std::string name="") {
        std::shared_ptr<Promise<T>> promise (new Promise<T>);
        promise->name = name;
        promise->self = promise;
        promise->setExecuter(executor, promise);
        return promise;
    }

    static shared_ptr success(T val) {
        shared_ptr promise (new Promise<T>(val));
        promise->self = promise;
        return promise;
    }

    static shared_ptr failure( Exception e) {
        shared_ptr promise (new Promise<T>(e));
        promise->self = promise;
        return promise;
    }

    ~Promise() {
    }


protected:
    //
    // Delivering Results
    //

    /**
     * Called when promise ended with error
     *
     * @param e error
     */
    void error(Exception e) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (isFinished) {
            throw std::runtime_error("Promise already completed!");
        }
        if (e == nullptr) {
            throw std::runtime_error("Error can't be null");
        }
        isFinished = true;
        exception = e;
        deliverResult();
        result_lock.reset();
    }

    /**
     * Trying complete promise with error
     *
     * @param e error
     */
    void tryError(Exception e) {
        if (isFinished) {
            return;
        }
        error(e);
    }

    /**
     * Called when result is ready
     *
     * @param res result
     */
    void result_(T res) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (isFinished) {
            throw std::runtime_error("Promise already completed!");
        }
        isFinished = true;
        result = std::make_shared<T>(res);
        deliverResult();
        result_lock.reset();
    }

    /**
     * Trying complete promise with result
     *
     * @param res result
     */
    void tryResult(T res) {

        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (!isFinished) {
            result_(res);
        }
    }

    /**
     * Delivering result
     */
    void deliverResult() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        shared_ptr self = nullptr;
        try {
        self = this->self.lock();
        } catch (...)
        {
            std::cerr<<"reference to this gone!"<<std::endl;
            result_lock.reset();
            callback_lock.reset();
            return;
        }
        if (callbacks.size() > 0) {
            dispatcher->dispatch([this, self]() {
                invokeDeliver(self);
            });
        }
    }

    void invokeDeliver(shared_ptr self) {
        std::lock_guard<std::recursive_mutex> lock(mutex);

        if (exception != nullptr) {
            for (auto& on_error : failures) {
                try {
                    on_error(exception);
                } catch (std::exception e) {
                    std::cerr<<e.what()<<std::endl;
                }
            }
        } else {
            for (auto& on_result : callbacks) {
                try {
                    on_result(*result);
                } catch (...) {
                    std::cerr<<"error"<<std::endl;
                }
            }

        }
        callbacks.clear();
        callback_lock.reset();
    }

public:

    /**
   * Handling failure
   *
   * @param failure supplier for exception
   * @return this
   */

    shared_ptr failure(OnError failure) {
        std::lock_guard<std::recursive_mutex> lock(mutex);

        if (isFinished) {
            if (exception != nullptr) {
                callback_lock = self.lock();
                dispatcher->dispatch([this, failure](){
                    failure(exception);
                    callback_lock.reset();
                });
            }
        } else {
            callback_lock = self.lock();
            callbacks.push_back([](T result){});
            failures.push_back(failure);
        }
        callback_lock = self.lock();

        if (callback_lock.get())
            return callback_lock;
        else {
            return self.lock();
        }
    }


    /**
    * Handling successful result
    *
    * @param then supplier for result
    * @return this
    */

    shared_ptr then(const OnResult then) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (isFinished) {
            if (exception == nullptr) {
                callback_lock = self.lock();
                dispatcher->dispatch([this, then](){
                    then(*result);
                    callback_lock.reset();
                });
            }
        } else {
            callback_lock = self.lock();
            callbacks.push_back(then);
            failures.push_back([](Exception ex){});

        }

        if (callback_lock.get())
            return callback_lock;
        else {
            return self.lock();
        }
    }

    /**
    * Mapping result value of promise to another value
    *
    * @param res mapping function
    * @param <R> destination type
    * @return promise
    */
    template<typename R>
    std::shared_ptr<Promise<R>> map( Function<R> res) {
        auto p = Promise<R>::createPromise([res, this](PromiseResolver<R> promiseResolver) {
            then([&, res, promiseResolver](T result) {
                auto resolver = const_cast<PromiseResolver<R>&> (promiseResolver);
                R r;
                try {
                    r = res(result);
                } catch (std::exception e) {
                    std::cerr<<e.what()<<std::endl;
                    resolver.tryError(std::make_shared<std::exception >(e));
                    return;
                }
                resolver.tryResult(r);
            });
            failure([&, promiseResolver](Exception ex){
                auto resolver = const_cast<PromiseResolver<R>&> (promiseResolver);
                resolver.error(ex);
            });

        });
        return p;
    }

    /**
     * Map result of promise to promise of value
     *
     * @param res mapping function
     * @param <R> destination type
     * @return promise
     */
    template<typename R>
    std::shared_ptr<Promise<R>> flatMap(FunctionPromise<R> res) {

        shared_ptr self = this->self.lock();
        auto p = Promise<R>::createPromise([res, this, &self](PromiseResolver<R> promiseResolver) {
            self->then([&, res, promiseResolver](T result) {
                auto resolver = const_cast<PromiseResolver<R>&> (promiseResolver);
                std::shared_ptr<Promise<R>> promise;
                try {
                    promise = res(result);
                } catch (std::exception e) {
                    resolver.tryError(std::make_shared<std::exception >(e));
                    return;
                }
                promise->then([&, resolver](R result2){
                    auto resolver_ = const_cast<PromiseResolver<R>&> (resolver);
                    resolver_.result(result2);
                });
                promise->failure([&, resolver](Exception ex){
                    auto resolver_ = const_cast<PromiseResolver<R>&> (resolver);
                    resolver_.error(ex);
                });
            });
            self->failure([&, promiseResolver](Exception ex){
                auto resolver = const_cast<PromiseResolver<R>&> (promiseResolver);
                resolver.error(ex);
            });

        }, "returnedPromise_"+name);

        return p;
    }

    /**
     * Chaining result to next promise and returning current value as result promise
     *
     * @param res chaining function
     * @param <R> destination type
     * @return promise
     */
    template<typename R>
    std::shared_ptr<Promise<T>> chain(FunctionPromise<R> res) {

        shared_ptr self = this->self.lock();
        auto p = Promise<T>::createPromise([res, this, &self](PromiseResolver<T> promiseResolver) {
            self->then([&, res, promiseResolver](T result) {
                auto resolver = const_cast<PromiseResolver<T>&> (promiseResolver);
                std::shared_ptr<Promise<R>> promise;
                try {
                    promise = res(result);
                } catch (std::exception e) {
                    resolver.tryError(std::make_shared<std::exception >(e));
                    return;
                }
                promise->then([&, resolver, result](R result2){
                    auto resolver_ = const_cast<PromiseResolver<T>&> (resolver);
                    resolver_.result(result);
                });
                promise->failure([&, resolver, result](Exception ex){
                    auto resolver_ = const_cast<PromiseResolver<T>&> (resolver);
                    resolver_.error(ex);
                });
            });
            self->failure([&, promiseResolver](Exception ex){
                auto resolver = const_cast<PromiseResolver<T>&> (promiseResolver);
                resolver.error(ex);
            });

        }, "returnedPromise_"+name);

        return p;
    }

    /**
    * Called after success or failure
    *
    * @param afterHandler after handler
    * @return this
    */
    shared_ptr after(OnAfter afterHandler) {
        return then([afterHandler](const T& t){afterHandler(std::make_shared<T>(t), nullptr);})
                ->
        failure([afterHandler](Exception ex){afterHandler(nullptr, ex);});
    }

};





#endif //PROMISEC11_PROMISE_HPP_H
