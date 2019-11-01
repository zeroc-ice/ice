//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class MyClassI final : public Test::MyClass, public std::enable_shared_from_this<MyClassI>
{
public:

    void callCallbackAsync(std::function<void()>,
                           std::function<void(std::exception_ptr)>,
                           const Ice::Current&) override;

    void getCallbackCountAsync(std::function<void(int)>,
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

    void getCallbackDatagramCountAsync(std::function<void(int)>,
                                       std::function<void(std::exception_ptr)>,
                                       const Ice::Current&) override;

    int getHeartbeatCount(const Ice::Current&) override;
    void enableHeartbeats(const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;

    void removeConnection(const std::shared_ptr<Ice::Connection>&);
    void incHeartbeatCount(const std::shared_ptr<Ice::Connection>&);

private:

    void checkConnection(const std::shared_ptr<Ice::Connection>&);

    std::mutex _mutex;
    std::condition_variable _condVar;
    int _datagramCount = 0;
    std::map<std::shared_ptr<Ice::Connection>, int> _connections;
    int _counter = 0;
};

#endif
