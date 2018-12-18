// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Test;

class CallbackBase : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    CallbackBase() :
        _called(false)
    {
    }

    virtual ~CallbackBase()
    {
    }

    void check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        while(!_called)
        {
            wait();
        }
        _called = false;
    }

protected:

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(!_called);
        _called = true;
        notify();
    }

private:

    bool _called;
};

class Callback : public virtual IceUtil::Shared, public virtual CallbackBase
{
public:

    void response()
    {
        test(false);
    }

    void exception(const Ice::Exception&)
    {
        test(false);
    }

    void opPidI(Ice::Int pid)
    {
        _pid = pid;
        called();
    }

    void opShutdownI()
    {
        called();
    }

    void exceptAbortI(const Ice::Exception& ex)
    {
        try
        {
            ex.ice_throw();
        }
        catch(const Ice::ConnectionLostException&)
        {
        }
        catch(const Ice::ConnectFailedException&)
        {
        }
        catch(const Ice::Exception& e)
        {
            cout << e << endl;
            test(false);
        }
        called();
    }

    Ice::Int pid() const
    {
        return _pid;
    }

private:

    Ice::Int _pid;
};
typedef IceUtil::Handle<Callback> CallbackPtr;

void
allTests(Test::TestHelper* helper, const vector<int>& ports)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    cout << "testing stringToProxy... " << flush;
    ostringstream ref;
    ref << "test";
    for(vector<int>::const_iterator p = ports.begin(); p != ports.end(); ++p)
    {
        ref << ":" << helper->getTestEndpoint(*p);
    }
    Ice::ObjectPrxPtr base = communicator->stringToProxy(ref.str());
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestIntfPrxPtr obj = ICE_CHECKED_CAST(TestIntfPrx, base);
    test(obj);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(obj, base));
#else
    test(obj == base);
#endif
    cout << "ok" << endl;

    int oldPid = 0;
    bool ami = false;
    for(unsigned int i = 1, j = 0; i <= ports.size(); ++i, ++j)
    {
        if(j > 3)
        {
            j = 0;
            ami = !ami;
        }

        if(!ami)
        {
            cout << "testing server #" << i << "... " << flush;
            int pid = obj->pid();
            test(pid != oldPid);
            cout << "ok" << endl;
            oldPid = pid;
        }
        else
        {
            cout << "testing server #" << i << " with AMI... " << flush;
#ifdef ICE_CPP11_MAPPING
            try
            {
                int pid = obj->pidAsync().get();
                test(pid != oldPid);
                cout << "ok" << endl;
                oldPid = pid;
            }
            catch(const exception&)
            {
                test(false);
            }
#else
            CallbackPtr cb = new Callback();
            obj->begin_pid(newCallback_TestIntf_pid(cb, &Callback::opPidI, &Callback::exception));
            cb->check();
            int pid = cb->pid();
            test(pid != oldPid);
            cout << "ok" << endl;
            oldPid = pid;
#endif
        }

        if(j == 0)
        {
            if(!ami)
            {
                cout << "shutting down server #" << i << "... " << flush;
                obj->shutdown();
                cout << "ok" << endl;
            }
            else
            {
#ifdef ICE_CPP11_MAPPING
                cout << "shutting down server #" << i << " with AMI... " << flush;
                try
                {
                    obj->shutdownAsync().get();
                }
                catch(const exception&)
                {
                    test(false);
                }
                cout << "ok" << endl;
#else
                cout << "shutting down server #" << i << " with AMI... " << flush;
                CallbackPtr cb = new Callback;
                obj->begin_shutdown(newCallback_TestIntf_shutdown(cb, &Callback::opShutdownI, &Callback::exception));
                cb->check();
                cout << "ok" << endl;
#endif
            }
        }
        else if(j == 1 || i + 1 > ports.size())
        {
            if(!ami)
            {
                cout << "aborting server #" << i << "... " << flush;
                try
                {
                    obj->abort();
                    test(false);
                }
                catch(const Ice::ConnectionLostException&)
                {
                    cout << "ok" << endl;
                }
                catch(const Ice::ConnectFailedException&)
                {
                    cout << "ok" << endl;
                }
            }
            else
            {
#ifdef ICE_CPP11_MAPPING
                cout << "aborting server #" << i << " with AMI... " << flush;
                try
                {
                    obj->abortAsync().get();
                    test(false);
                }
                catch(const exception&)
                {
                }
                cout << "ok" << endl;
#else
                cout << "aborting server #" << i << " with AMI... " << flush;
                CallbackPtr cb = new Callback;
                obj->begin_abort(newCallback_TestIntf_abort(cb, &Callback::response, &Callback::exceptAbortI));
                cb->check();
                cout << "ok" << endl;
#endif
            }
        }
        else if(j == 2 || j == 3)
        {
            if(!ami)
            {
                cout << "aborting server #" << i << " and #" << i + 1 << " with idempotent call... " << flush;
                try
                {
                    obj->idempotentAbort();
                    test(false);
                }
                catch(const Ice::ConnectionLostException&)
                {
                    cout << "ok" << endl;
                }
                catch(const Ice::ConnectFailedException&)
                {
                    cout << "ok" << endl;
                }
            }
            else
            {
#ifdef ICE_CPP11_MAPPING
                cout << "aborting server #" << i << " and #" << i + 1 << " with idempotent AMI call... " << flush;
                try
                {
                    obj->idempotentAbortAsync().get();
                    test(false);
                }
                catch(const exception&)
                {
                }
                cout << "ok" << endl;
#else
                cout << "aborting server #" << i << " and #" << i + 1 << " with idempotent AMI call... " << flush;
                CallbackPtr cb = new Callback;
                obj->begin_idempotentAbort(newCallback_TestIntf_idempotentAbort(cb, &Callback::response,
                                                                                &Callback::exceptAbortI));
                cb->check();
                cout << "ok" << endl;
#endif
            }

            ++i;
        }
        else
        {
            assert(false);
        }
    }

    cout << "testing whether all servers are gone... " << flush;
    try
    {
        obj->ice_ping();
        test(false);
    }
    catch(const Ice::LocalException&)
    {
        cout << "ok" << endl;
    }
}
