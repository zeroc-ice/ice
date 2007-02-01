// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
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

    bool check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        while(!_called)
        {
            if(!timedWait(IceUtil::Time::seconds(30)))
            {
                return false;
            }
        }
        _called = false;
        return true;
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

class AMI_Test_pidI : virtual public AMI_TestIntf_pid, virtual public CallbackBase
{
public:

    virtual void ice_response(Ice::Int pid)
    {
        _pid = pid;
        called();
    }

    virtual void ice_exception(const Ice::Exception& ex)
    {
        test(false);
    }

    Ice::Int pid() const
    {
        return _pid;
    }

private:

    Ice::Int _pid;
};

typedef IceUtil::Handle<AMI_Test_pidI> AMI_Test_pidIPtr;

class AMI_Test_shutdownI : virtual public AMI_TestIntf_shutdown, virtual public CallbackBase
{
public:

    virtual void ice_response()
    {
        called();
    }

    virtual void ice_exception(const Ice::Exception&)
    {
        test(false);
    }
};

typedef IceUtil::Handle<AMI_Test_shutdownI> AMI_Test_shutdownIPtr;

class AMI_Test_abortI : virtual public AMI_TestIntf_abort, virtual public CallbackBase
{
public:

    virtual void ice_response()
    {
        test(false);
    }

    virtual void ice_exception(const Ice::Exception& ex)
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
        catch(Ice::Exception& ex)
        {
            cout << ex << endl;
            test(false);
        }
        called();
    }
};

typedef IceUtil::Handle<AMI_Test_abortI> AMI_Test_abortIPtr;

class AMI_Test_idempotentAbortI : public AMI_TestIntf_idempotentAbort, public CallbackBase
{
    virtual void ice_response()
    {
        test(false);
    }

    virtual void ice_exception(const Ice::Exception& ex)
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
        catch(Ice::Exception& ex)
        {
            cout << ex << endl;
            test(false);
        }
        called();
    }
};

typedef IceUtil::Handle<AMI_Test_idempotentAbortI> AMI_Test_idempotentAbortIPtr;

void
allTests(const Ice::CommunicatorPtr& communicator, const vector<int>& ports)
{
    cout << "testing stringToProxy... " << flush;
    ostringstream ref;
    ref << "test";
    for(vector<int>::const_iterator p = ports.begin(); p != ports.end(); ++p)
    {
        ref << ":default -t 60000 -p " << *p;
    }
    Ice::ObjectPrx base = communicator->stringToProxy(ref.str());
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestIntfPrx obj = TestIntfPrx::checkedCast(base);
    test(obj);
    test(obj == base);
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
            AMI_Test_pidIPtr cb = new AMI_Test_pidI();
            obj->pid_async(cb);
            test(cb->check());
            int pid = cb->pid();
            test(pid != oldPid);
            cout << "ok" << endl;
            oldPid = pid;
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
                cout << "shutting down server #" << i << " with AMI... " << flush;
                AMI_Test_shutdownIPtr cb = new AMI_Test_shutdownI;
                obj->shutdown_async(cb);
                test(cb->check());
                cout << "ok" << endl;
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
                cout << "aborting server #" << i << " with AMI... " << flush;
                AMI_Test_abortIPtr cb = new AMI_Test_abortI;
                obj->abort_async(cb);
                test(cb->check());
                cout << "ok" << endl;
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
                cout << "aborting server #" << i << " and #" << i + 1 << " with idempotent AMI call... " << flush;
                AMI_Test_idempotentAbortIPtr cb = new AMI_Test_idempotentAbortI;
                obj->idempotentAbort_async(cb);
                test(cb->check());
                cout << "ok" << endl;
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
