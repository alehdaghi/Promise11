
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

    std::cout<<"hello "<<std::endl;
    std::vector<Promise<std::string>::Supplier> list;
    for (int i=0;i<4;i++)
    {
        list.push_back([i](){
            return tempTask3(i)->then([i](std::string a){
                std::cout<<"binary of "<<i<<" is: "<<a<<std::endl;
        });
        });
    }
    Promise<std::string>::traverse(list)->then([](){
        std::cout<<"here all tasks is done!"<<std::endl;
    });
    std::cout<<"world! "<<std::endl;

//    auto p = tempTask3(10)->then([](std::string res){
//        sleep(1);
//        std::cout<<"then 1, res: "<<res<<std::endl;
//    })->then([](std::string res){
//        sleep(1);
//        std::cout<<"then 2, res: "<<res<<std::endl;
//    })->then([](std::string res){
//        sleep(0);
//        std::cout<<"then 3, res: "<<res<<std::endl;
//    })->then([](std::string res){
//        sleep(1);
//        std::cout<<"then 4, res: "<<res<<std::endl;
//    })->then([](std::string res){
//        sleep(0);
//        std::cout<<"then 5, res: "<<res<<std::endl;
//    })
//    ->flatMap<int>([](std::string res) {
//        std::cout<<"flatMap 1: "<<res<<std::endl;
//        return tempTask2(res);
//    })->then([](int a){
//        std::cout<<"then 6, a: "<<a<<std::endl;
//    })
//    ->failure([](std::shared_ptr<std::exception> ex){
//        std::cerr<<"fail 1, what: "<<ex->what()<<std::endl;
//    })->then([](int a){
//                std::cout<<"then 7, a: "<<a<<std::endl;
//    })->chain<std::string>([](int a){
//        std::cout<<"chain1: "<<a<<std::endl;
//        return tempTask3(a);
//    })
//
//    ;
//
//    p->then([](int a){
//        std::cout<<"then 8, a: "<<a<<std::endl;
//    });
//
//    p->failure([](std::shared_ptr<std::exception> ex){
//        std::cerr<<"fail 2, what: "<<ex->what()<<std::endl;
//    });
//
//
//    auto p2 = Promise<int>::success(100);
//    auto p3 = p2->then([](int a){
//        std::cout<<"then 9, a: "<<a<<std::endl;
//    })->flatMap<std::string>([](int a){
//        std::cout<<"flatMap 2: "<<a<<std::endl;
//        return tempTask3(a);
//    })->after([](std::shared_ptr<std::string> res, std::shared_ptr<std::exception> ex){
//        if (res != nullptr)
//            std::cout<<"after 1, result: "<<*res<<std::endl;
//        else
//            std::cout<<"after 1, error: "<<ex->what()<<std::endl;
//    })->then([](std::string res){
//        std::cout<<"then 10, a: "<<res<<std::endl;
//    });



    while(true) {
        sleep(1);
    }

    return 0;
}
