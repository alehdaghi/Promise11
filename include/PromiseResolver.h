//
// Created by mahdi on 10/15/17.
//

#ifndef PROMISEC11_PROMISERESOLVER_H
#define PROMISEC11_PROMISERESOLVER_H


#include "promise.h"

template<typename T> class Promise;

template<typename T>
class PromiseResolver {

    using shared_ptr = typename Promise<T>::shared_ptr;
    shared_ptr promise;

public:

    PromiseResolver(shared_ptr promise) : promise(promise) {
    }

//    PromiseResolver(std::shared_ptr<Promise<T>> promise) {
//        std::cout<<"PromiseResolver"<<" addr: "<< promise <<std::endl;
//        this->promise = promise;
//    }

    PromiseResolver(const PromiseResolver& resolver) {
        promise = resolver.promise;
    }

    /**
     * Get Resolver's promise
     *
     * @return promise
     */

    shared_ptr getPromise() {
        return promise;
    }

    /**
     * Call this to complete promise
     *
     * @param res result of promise
     */

    void result(T res) {
        promise->result_(res);

    }

    /**
     * Trying to complete promise
     *
     * @param res result of promise
     */

    void tryResult( T res) {
        promise->tryResult(res);
    }

    /**
     * Call this to fail promise
     *
     * @param e reason
     */

    void error( typename Promise<T>::Exception e) {
        promise->error(e);
    }

    /**
     * Trying to fail promise
     *
     * @param e reason
     */

    void tryError(typename Promise<T>::Exception e) {
        promise->tryError(e);
    }

    ~PromiseResolver() {
    }

};


#endif //PROMISEC11_PROMISERESOLVER_H
