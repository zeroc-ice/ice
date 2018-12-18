// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class MyClassI : public Test::MyClass
{
public:

    MyClassI();

    virtual void callCallback_async(const Test::AMD_MyClass_callCallbackPtr&, const Ice::Current&);
    virtual void getCallbackCount_async(const Test::AMD_MyClass_getCallbackCountPtr&, const Ice::Current&);

    virtual void incCounter(int, const Ice::Current&);
    virtual void waitCounter(int, const Ice::Current&);

    virtual int getConnectionCount(const Ice::Current&);
    virtual std::string getConnectionInfo(const Ice::Current&);
    virtual void closeConnection(bool, const Ice::Current&);

    virtual void datagram(const Ice::Current&);
    virtual int getDatagramCount(const Ice::Current&);

    virtual void callDatagramCallback(const Ice::Current&);
    virtual void getCallbackDatagramCount_async(const Test::AMD_MyClass_getCallbackDatagramCountPtr&, const Ice::Current&);

    virtual int getHeartbeatCount(const Ice::Current&);
    virtual void enableHeartbeats(const Ice::Current&);

    virtual void shutdown(const Ice::Current&);

    void removeConnection(const Ice::ConnectionPtr&);
    void incHeartbeatCount(const Ice::ConnectionPtr&);

private:

    void checkConnection(const Ice::ConnectionPtr&);

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    int _datagramCount;
    std::map<Ice::ConnectionPtr, int> _connections;
    int _counter;
};
typedef IceUtil::Handle<MyClassI> MyClassIPtr;

#endif
