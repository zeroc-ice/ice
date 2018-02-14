// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <MyObjectI.h>
#include <TestCommon.h>
#include <IceUtil/IceUtil.h>

using namespace IceUtil; 
using namespace std;

MySystemException::MySystemException(const char* file, int line) :
    Ice::SystemException(file, line)
{
}

MySystemException::~MySystemException() throw()
{
}

string
MySystemException::ice_name() const
{
    return "MySystemException";
}

MySystemException*
MySystemException::ice_clone() const
{
    return new MySystemException(*this);
}

void
MySystemException::ice_throw() const
{
    throw *this;
}

int 
MyObjectI::add(int x, int y, const Ice::Current&)
{
    return x + y;
}

int 
MyObjectI::addWithRetry(int x, int y, const Ice::Current& current)
{
    Ice::Context::const_iterator p = current.ctx.find("retry");
    
    if(p == current.ctx.end() || p->second != "no")
    {
        throw Test::RetryException(__FILE__, __LINE__);
    }
    return x + y;
}

int 
MyObjectI::badAdd(int, int, const Ice::Current&)
{
    throw Test::InvalidInputException();
}

int 
MyObjectI::notExistAdd(int, int, const Ice::Current&)
{
    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
}

int 
MyObjectI::badSystemAdd(int, int, const Ice::Current&)
{
    throw MySystemException(__FILE__, __LINE__);
}
    

void 
MyObjectI::amdAdd_async(const Test::AMD_MyObject_amdAddPtr& cb, int x, int y, const Ice::Current&)
{
    class ThreadI : public Thread
    {
    public:
        
        ThreadI(const Test::AMD_MyObject_amdAddPtr& cb, int x, int y) :
            _cb(cb),
            _x(x),
            _y(y)
        {
        }

        void run()
        {
            ThreadControl::sleep(Time::milliSeconds(10));
            _cb->ice_response(_x + _y);
        }
    private:
        Test::AMD_MyObject_amdAddPtr _cb;
        int _x;
        int _y;
    };

    ThreadPtr thread = new ThreadI(cb, x, y);
    thread->start().detach();
}

void 
MyObjectI::amdAddWithRetry_async(const Test::AMD_MyObject_amdAddWithRetryPtr& cb, int x, int y, const Ice::Current& current)
{
    class ThreadI : public Thread
    {
    public:
        
        ThreadI(const Test::AMD_MyObject_amdAddWithRetryPtr& cb, int x, int y) :
            _cb(cb),
            _x(x),
            _y(y)
        {
        }

        void run()
        {
            ThreadControl::sleep(Time::milliSeconds(10));
            _cb->ice_response(_x + _y);
        }
    private:
        Test::AMD_MyObject_amdAddWithRetryPtr _cb;
        int _x;
        int _y;
    };

    ThreadPtr thread = new ThreadI(cb, x, y);
    thread->start().detach();

    Ice::Context::const_iterator p = current.ctx.find("retry");
    
    if(p == current.ctx.end() || p->second != "no")
    {
        throw Test::RetryException(__FILE__, __LINE__);
    }
}

void 
MyObjectI::amdBadAdd_async(const Test::AMD_MyObject_amdBadAddPtr& cb, int, int, const Ice::Current&)
{
    class ThreadI : public Thread
    {
    public:
        
        ThreadI(const Test::AMD_MyObject_amdBadAddPtr& cb) :
            _cb(cb)
        {
        }

        void run()
        {
            ThreadControl::sleep(Time::milliSeconds(10));
            Test::InvalidInputException e;
            _cb->ice_exception(e);
        }
    private:
        Test::AMD_MyObject_amdBadAddPtr _cb;
    };

    ThreadPtr thread = new ThreadI(cb);
    thread->start().detach();
}

void 
MyObjectI::amdNotExistAdd_async(const Test::AMD_MyObject_amdNotExistAddPtr& cb, int, int, const Ice::Current&)
{
    class ThreadI : public Thread
    {
    public:
        
        ThreadI(const Test::AMD_MyObject_amdNotExistAddPtr& cb) :
            _cb(cb)
        {
        }

        void run()
        {
            ThreadControl::sleep(Time::milliSeconds(10));
            _cb->ice_exception(Ice::ObjectNotExistException(__FILE__, __LINE__));
        }
    private:
        Test::AMD_MyObject_amdNotExistAddPtr _cb;
    };

    ThreadPtr thread = new ThreadI(cb);
    thread->start().detach();
}

void 
MyObjectI::amdBadSystemAdd_async(const Test::AMD_MyObject_amdBadSystemAddPtr& cb, int, int, const Ice::Current&)
{
    class ThreadI : public Thread
    {
    public:
        
        ThreadI(const Test::AMD_MyObject_amdBadSystemAddPtr& cb) :
            _cb(cb)
        {
        }

        void run()
        {
            ThreadControl::sleep(Time::milliSeconds(10));
            _cb->ice_exception(MySystemException(__FILE__, __LINE__));
        }
    private:
        Test::AMD_MyObject_amdBadSystemAddPtr _cb;
    };

    ThreadPtr thread = new ThreadI(cb);
    thread->start().detach();
}
