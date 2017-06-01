// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

string
MySystemException::ice_id() const
{
    return "::MySystemException";
}

#ifdef ICE_CPP11_MAPPING
IceUtil::Exception*
MySystemException::ice_cloneImpl() const
{
    return new MySystemException(*this);
}
#else
MySystemException*
MySystemException::ice_clone() const
{
    return new MySystemException(*this);
}
#endif

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

#ifdef ICE_CPP11_MAPPING
void
MyObjectI::amdAddAsync(int x,
                       int y,
                       function<void(int)> response,
                       function<void(exception_ptr)>,
                       const Ice::Current&)
{
    thread t(
        [x, y, response]()
        {
            this_thread::sleep_for(chrono::milliseconds(10));
            response(x + y);
        });
    t.detach();
}

void
MyObjectI::amdAddWithRetryAsync(int x,
                                int y,
                                function<void(int)> response,
                                function<void(exception_ptr)>,
                                const Ice::Current& current)
{
    thread t(
        [x, y, response]()
        {
            try
            {
                this_thread::sleep_for(chrono::milliseconds(10));
                response(x + y);
            }
            catch(Ice::ResponseSentException&)
            {
            }
        });
    t.detach();

    Ice::Context::const_iterator p = current.ctx.find("retry");

    if(p == current.ctx.end() || p->second != "no")
    {
        throw Test::RetryException(__FILE__, __LINE__);
    }
}

void
MyObjectI::amdBadAddAsync(int x,
                          int y,
                          function<void(int)>,
                          function<void(exception_ptr)> error,
                          const Ice::Current&)
{
    thread t(
        [x, y, error]()
        {
            this_thread::sleep_for(chrono::milliseconds(10));
            try
            {
                throw Test::InvalidInputException();
            }
            catch(...)
            {
                error(current_exception());
            }
        });
    t.detach();
}

void
MyObjectI::amdNotExistAddAsync(int x,
                               int y,
                               function<void(int)>,
                               function<void(exception_ptr)> error,
                               const Ice::Current&)
{
    thread t(
        [x, y, error]()
        {
            this_thread::sleep_for(chrono::milliseconds(10));
            try
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
            }
            catch(...)
            {
                error(current_exception());
            }
        });
    t.detach();
}

void
MyObjectI::amdBadSystemAddAsync(int x,
                                int y,
                                function<void(int)>,
                                function<void(exception_ptr)> error,
                                const Ice::Current&)
{
    thread t(
        [x, y, error]()
        {
            this_thread::sleep_for(chrono::milliseconds(10));
            try
            {
                throw MySystemException(__FILE__, __LINE__);
            }
            catch(...)
            {
                error(current_exception());
            }
        });
    t.detach();
}
#else
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
        cb->ice_exception(Test::RetryException(__FILE__, __LINE__));
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
#endif
