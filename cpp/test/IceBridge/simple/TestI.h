// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class MyClassI final : public Test::MyClass, public std::enable_shared_from_this<MyClassI>
{
public:
    void
    callCallbackAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void getCallbackCountAsync(
        std::function<void(int)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current& current) override;

    void incCounter(int, const Ice::Current&) override;
    void waitCounter(int, const Ice::Current&) override;

    int getConnectionCount(const Ice::Current&) override;
    std::string getConnectionInfo(const Ice::Current&) override;
    void closeConnection(bool, const Ice::Current&) override;

    void datagram(const Ice::Current&) override;
    int getDatagramCount(const Ice::Current&) override;

    void callDatagramCallback(const Ice::Current&) override;

    void getCallbackDatagramCountAsync(
        std::function<void(int)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;

    void removeConnection(const Ice::ConnectionPtr&);

private:
    void checkConnection(const Ice::ConnectionPtr&);

    std::mutex _lock;
    std::condition_variable _condVar;
    int _datagramCount = 0;
    std::map<Ice::ConnectionPtr, int> _connections;
    int _counter = 0;
};

#endif
