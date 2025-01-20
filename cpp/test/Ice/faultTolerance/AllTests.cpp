// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;

void
allTests(Test::TestHelper* helper, const vector<int>& ports)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    ostringstream ref;
    ref << "test";
    for (int port : ports)
    {
        ref << ":" << helper->getTestEndpoint(port);
    }
    TestIntfPrx obj(communicator, ref.str());

    int oldPid = 0;
    bool ami = false;
    for (unsigned int i = 1, j = 0; i <= ports.size(); ++i, ++j)
    {
        if (j > 3)
        {
            j = 0;
            ami = !ami;
        }

        if (!ami)
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
            try
            {
                int pid = obj->pidAsync().get();
                test(pid != oldPid);
                cout << "ok" << endl;
                oldPid = pid;
            }
            catch (const exception&)
            {
                test(false);
            }
        }

        if (j == 0)
        {
            if (!ami)
            {
                cout << "shutting down server #" << i << "... " << flush;
                obj->shutdown();
                cout << "ok" << endl;
            }
            else
            {
                cout << "shutting down server #" << i << " with AMI... " << flush;
                try
                {
                    obj->shutdownAsync().get();
                }
                catch (const exception&)
                {
                    test(false);
                }
                cout << "ok" << endl;
            }
        }
        else if (j == 1 || i + 1 > ports.size())
        {
            if (!ami)
            {
                cout << "aborting server #" << i << "... " << flush;
                try
                {
                    obj->abort();
                    test(false);
                }
                catch (const Ice::ConnectionLostException&)
                {
                    cout << "ok" << endl;
                }
                catch (const Ice::ConnectFailedException&)
                {
                    cout << "ok" << endl;
                }
            }
            else
            {
                cout << "aborting server #" << i << " with AMI... " << flush;
                try
                {
                    obj->abortAsync().get();
                    test(false);
                }
                catch (const exception&)
                {
                }
                cout << "ok" << endl;
            }
        }
        else if (j == 2 || j == 3)
        {
            if (!ami)
            {
                cout << "aborting server #" << i << " and #" << i + 1 << " with idempotent call... " << flush;
                try
                {
                    obj->idempotentAbort();
                    test(false);
                }
                catch (const Ice::ConnectionLostException&)
                {
                    cout << "ok" << endl;
                }
                catch (const Ice::ConnectFailedException&)
                {
                    cout << "ok" << endl;
                }
            }
            else
            {
                cout << "aborting server #" << i << " and #" << i + 1 << " with idempotent AMI call... " << flush;
                try
                {
                    obj->idempotentAbortAsync().get();
                    test(false);
                }
                catch (const exception&)
                {
                }
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
    catch (const Ice::LocalException&)
    {
        cout << "ok" << endl;
    }
}
