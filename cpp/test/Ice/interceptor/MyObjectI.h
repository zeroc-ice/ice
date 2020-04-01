//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef MY_OBJECT_I_H
#define MY_OBJECT_I_H

#include <Test.h>

class MySystemException : public Ice::SystemException
{
public:

    MySystemException(const char*, int);

    virtual std::string ice_id() const;

    virtual void ice_throw() const;

protected:

    virtual IceUtil::Exception* ice_cloneImpl() const;
};

class MyObjectI : public Test::MyObject
{
public:

    virtual int add(int, int, const Ice::Current&);
    virtual int addWithRetry(int, int, const Ice::Current&);
    virtual int badAdd(int, int, const Ice::Current&);
    virtual int notExistAdd(int, int, const Ice::Current&);
    virtual int badSystemAdd(int, int, const Ice::Current&);

    virtual void amdAddAsync(int, int, std::function<void(int)>, std::function<void(std::exception_ptr)>, const Ice::Current&);
    virtual void amdAddWithRetryAsync(int, int, std::function<void(int)>, std::function<void(std::exception_ptr)>, const Ice::Current&);
    virtual void amdBadAddAsync(int, int, std::function<void(int)>, std::function<void(std::exception_ptr)>, const Ice::Current&);
    virtual void amdNotExistAddAsync(int, int, std::function<void(int)>, std::function<void(std::exception_ptr)>, const Ice::Current&);
    virtual void amdBadSystemAddAsync(int, int, std::function<void(int)>, std::function<void(std::exception_ptr)>, const Ice::Current&);
};

#endif
