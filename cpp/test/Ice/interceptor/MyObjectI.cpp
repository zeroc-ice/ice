//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <MyObjectI.h>
#include <TestHelper.h>
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
                       const Ice::Current& current)
{
    Ice::Context::const_iterator p = current.ctx.find("retry");
    bool retry = p != current.ctx.end();
    std::thread t(
        [x, y, response, retry]()
        {
            this_thread::sleep_for(chrono::milliseconds(10));
            try
            {
                response(x + y);
            }
            catch(const Ice::ResponseSentException&)
            {
                test(retry);
            }
        });
    t.detach();
}

void
MyObjectI::amdAddWithRetryAsync(int x,
                                int y,
                                function<void(int)> response,
                                function<void(exception_ptr)> error,
                                const Ice::Current& current)
{
    std::thread t(
        [x, y, response]()
        {
            try
            {
                this_thread::sleep_for(chrono::milliseconds(10));
                response(x + y);
            }
            catch(const Ice::ResponseSentException&)
            {
            }
        });
    t.detach();

    Ice::Context::const_iterator p = current.ctx.find("retry");

    if(p == current.ctx.end() || p->second != "no")
    {
        try
        {
            throw Test::RetryException(__FILE__, __LINE__);
        }
        catch(...)
        {
            error(std::current_exception());
        }
    }
}

void
MyObjectI::amdBadAddAsync(int,
                          int,
                          function<void(int)>,
                          function<void(exception_ptr)> error,
                          const Ice::Current&)
{
    std::thread t(
        [error]()
        {
            this_thread::sleep_for(chrono::milliseconds(10));
            try
            {
                throw Test::InvalidInputException();
            }
            catch(...)
            {
                error(std::current_exception());
            }
        });
    t.detach();
}

void
MyObjectI::amdNotExistAddAsync(int,
                               int,
                               function<void(int)>,
                               function<void(exception_ptr)> error,
                               const Ice::Current&)
{
    std::thread t(
        [error]()
        {
            this_thread::sleep_for(chrono::milliseconds(10));
            try
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
            }
            catch(...)
            {
                error(std::current_exception());
            }
        });
    t.detach();
}

void
MyObjectI::amdBadSystemAddAsync(int,
                                int,
                                function<void(int)>,
                                function<void(exception_ptr)> error,
                                const Ice::Current&)
{
    std::thread t(
        [error]()
        {
            this_thread::sleep_for(chrono::milliseconds(10));
            try
            {
                throw MySystemException(__FILE__, __LINE__);
            }
            catch(...)
            {
                error(std::current_exception());
            }
        });
    t.detach();
}
#else
void
MyObjectI::amdAdd_async(const Test::AMD_MyObject_amdAddPtr& cb, int x, int y, const Ice::Current& current)
{
    class ThreadI : public Thread
    {
    public:

        ThreadI(const Test::AMD_MyObject_amdAddPtr& cb, int x, int y, bool retry) :
            _cb(cb),
            _x(x),
            _y(y),
            _retry(retry)
        {
        }

        void run()
        {
            ThreadControl::sleep(Time::milliSeconds(10));
            try
            {
                _cb->ice_response(_x + _y);
            }
            catch(const Ice::ResponseSentException&)
            {
                test(_retry);
            }
        }
    private:
        Test::AMD_MyObject_amdAddPtr _cb;
        int _x;
        int _y;
        bool _retry;
    };

    Ice::Context::const_iterator p = current.ctx.find("retry");
    bool retry = p != current.ctx.end();
    ThreadPtr thread = new ThreadI(cb, x, y, retry);
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
