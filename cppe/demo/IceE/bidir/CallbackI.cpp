// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <CallbackI.h>

using namespace std;
using namespace Demo;

CallbackSenderI::CallbackSenderI(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator),
    _destroy(false),
    _callbackSenderThread(new CallbackSenderThread(this))
{
}

void
CallbackSenderI::destroy()
{
    IceUtil::ThreadPtr callbackSenderThread;

    {
        Lock lock(*this);
        
        printf("destroying callback sender\n");
        _destroy = true;
        
        notify();

        callbackSenderThread = _callbackSenderThread;
        _callbackSenderThread = 0; // Resolve cyclic dependency.
    }

    callbackSenderThread->getThreadControl().join();
}

void
CallbackSenderI::addClient(const Ice::Identity& ident, const Ice::Current& current)
{
    Lock lock(*this);

    printf("adding client `%s'\n", _communicator->identityToString(ident).c_str());

    CallbackReceiverPrx client = CallbackReceiverPrx::uncheckedCast(current.con->createProxy(ident));
    _clients.insert(client);
}

void
CallbackSenderI::start()
{
    _callbackSenderThread->start();
}

void
CallbackSenderI::run()
{
    int num = 0;
    while(true)
    {
        std::set<Demo::CallbackReceiverPrx> clients;
        {
            Lock lock(*this);
            timedWait(IceUtil::Time::seconds(2));

            if(_destroy)
            {
                break;
            }

            clients = _clients;
        }

        if(!clients.empty())
        {
            ++num;
            for(set<CallbackReceiverPrx>::iterator p = clients.begin(); p != clients.end(); ++p)
            {
                try
                {
                    (*p)->callback(num);
                }
                catch(const Ice::Exception& ex)
                {
                    fprintf(stderr, "removing client `%s':\n%s\n",
                            _communicator->identityToString((*p)->ice_getIdentity()).c_str(),
                                ex.toString().c_str());
                    _clients.erase(*p);
                }
            }
        }
    }
}
