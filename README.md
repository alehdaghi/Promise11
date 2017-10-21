# Promise11
A new promise (and future) in C++ 11

Advanced promise with map, flat map and chain ability.

Promise can be resulted async, and its then callback also be runned in other thread
```C++
std::string str = "mahdi";
auto/*== std::shared_ptr<Promise<int>>*/ p = Promise<int>::createPromise(
            [str](PromiseResolver<int> pr) {
                dispatcher->dispatch([&, pr, str]// dispatch a task on other thread
                     {
                         auto resolver = const_cast<PromiseResolver<int>&> (pr);
                         try {
                             int n = syncTask2(str);// process on str that we wnat to run on other thread
                             resolver.result(n);
                         }
                         catch (std::exception ex)
                         {
                             std::cerr<<ex.what()<<std::endl;
                             resolver.tryError(std::make_shared<std::exception >(ex));
                         }
                     }
                );
            })
p->then([](int result){
  std::cout<<"result in then 1 :"<<res<<std::endl;// this thread may be different from thread that creates and sets result
})
->then([](int result){
  std::cout<<"result in then 2 :"<<res<<std::endl;// this thread may be different from thread that creates and sets result but as same as recent then
})
->failure([](std::shared_ptr<std::exception> ex) {
  std::cerr<<"ex: "<< ex->what()<<std::endl;
});

std::shared_ptr<Promise<std::string>> p2 = p->map<std::string>([](int result){
  return std::bitset<8>(result).to_string(); // convert an int to binary
});

p2->then([](string result){
  std::cout<<"binary of result of p is :"<<res<<std::endl;
})


```

Full example is here
```C++

#include <iostream>
#include <Promise.h>
#include <future>
#include <zconf.h>
#include <bitset>

std::string syncTask1(int a){ // calculate binary of int by string
    sleep(2);// simulate time consuming
    return std::bitset<8>(a).to_string();;
}

int syncTask2(std::string str){ // calculate the size of a string
    sleep(3);// simulate time consuming
    return str.size();
}


std::shared_ptr<AsyncDispatcher> dispatcher = std::shared_ptr<AsyncDispatcher>(new AsyncDispatcher);

std::shared_ptr<Promise<std::string>> tempTask1(int a) {// do syncTask1 async
// doing a task async and returning promise
    std::cout<<"task1"<<std::endl;
    return Promise<std::string>::createPromise(
            [a](PromiseResolver<std::string> pr) {
                dispatcher->dispatch([&, pr, a]
                {
                    auto resolver = const_cast<PromiseResolver<std::string>&> (pr);
                    try {
                        std::string s = syncTask1(a);
                        resolver.result(s);
                    }
                    catch (std::exception ex)
                    {
                        std::cerr<<ex.what()<<std::endl;
                        resolver.tryError(std::make_shared<std::exception >(ex));
                    }
                }
                );
            }, "asyncTaskPromise1")
    ;
}

std::shared_ptr<Promise<int>> tempTask2(std::string str) { //do syncTask2 async
// doing a task async and returning promise
    return Promise<int>::createPromise(
            [str](PromiseResolver<int> pr) {
                dispatcher->dispatch([&, pr, str]
                     {
                         auto resolver = const_cast<PromiseResolver<int>&> (pr);
                         try {
                             int n = syncTask2(str);
                             resolver.result(n);
                         }
                         catch (std::exception ex)
                         {
                             std::cerr<<ex.what()<<std::endl;
                             resolver.tryError(std::make_shared<std::exception >(ex));
                         }
                     }
                );
            }, "asyncTaskPromise2")
    ;
}

std::shared_ptr<Promise<std::string>> tempTask3(int a) {// do syncTask1 async
// doing a task async and returning promise
    std::cout<<"task3"<<std::endl;
    return Promise<std::string>::createPromise(
            [a](PromiseResolver<std::string> pr) {
                dispatcher->dispatch([&, pr, a]
                                     {
                                         auto resolver = const_cast<PromiseResolver<std::string>&> (pr);
                                         try {
                                             std::string s = syncTask1(a);
                                             resolver.result(s);
                                         }
                                         catch (std::exception ex)
                                         {
                                             resolver.tryError(std::make_shared<std::exception >(ex));
                                         }
                                     }
                );
            }, "asyncTaskPromise1");
}


int main() {


    ThreadDispatcher::pushDispatcher
            (std::shared_ptr<AsyncDispatcher>(new AsyncDispatcher));


    auto p = tempTask3(10)->then([](std::string res){
        sleep(1);
        std::cout<<"then 1, res: "<<res<<std::endl;
    })->then([](std::string res){
        sleep(1);
        std::cout<<"then 2, res: "<<res<<std::endl;
    })->then([](std::string res){
        sleep(0);
        std::cout<<"then 3, res: "<<res<<std::endl;
    })->then([](std::string res){
        sleep(1);
        std::cout<<"then 4, res: "<<res<<std::endl;
    })->then([](std::string res){
        sleep(0);
        std::cout<<"then 5, res: "<<res<<std::endl;
    })
    ->flatMap<int>([](std::string res) {
        std::cout<<"flatMap 1: "<<res<<std::endl;
        return tempTask2(res);
    })->then([](int a){
        std::cout<<"then 6, a: "<<a<<std::endl;
    })
    ->failure([](std::shared_ptr<std::exception> ex){
        std::cerr<<"fail 1, what: "<<ex->what()<<std::endl;
    })->then([](int a){
                std::cout<<"then 7, a: "<<a<<std::endl;
    })->chain<std::string>([](int a){
        std::cout<<"chain1: "<<a<<std::endl;
        return tempTask3(a);
    })

    ;

    p->then([](int a){
        std::cout<<"then 8, a: "<<a<<std::endl;
    });

    p->failure([](std::shared_ptr<std::exception> ex){
        std::cerr<<"fail 2, what: "<<ex->what()<<std::endl;
    });


    auto p2 = Promise<int>::success(100);
    auto p3 = p2->then([](int a){
        std::cout<<"then 9, a: "<<a<<std::endl;
    })->flatMap<std::string>([](int a){
        std::cout<<"flatMap 2: "<<a<<std::endl;
        return tempTask3(a);
    })->then([](std::string res){
        std::cout<<"then 10, a: "<<res<<std::endl;
    });
    std::cout<<"hi !!!!"<<std::endl;


    while(true) {
        sleep(1);
    }

    return 0;
}

```
and its result is:
```
  task3
hi !!!!
then 9, a: 100
flatMap 2: 100
task3
then 1, res: 00001010
then 2, res: 00001010
then 3, res: 00001010
then 4, res: 00001010
then 5, res: 00001010
flatMap 1: 00001010
then 10, a: 01100100
then 6, a: 8
then 7, a: 8
chain1: 8
task3
then 8, a: 8
 ```
