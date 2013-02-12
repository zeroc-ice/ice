// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <CallbackI.h>

#if defined(_MSC_VER) && (_MSC_VER < 1700)
#    pragma warning( 4 : 4355 ) // C4355 'this' : used in base member initializer list
#endif

using namespace std;
using namespace Ice;
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
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
        
        cout << "destroying callback sender" << endl;
        _destroy = true;
        
        notify();

        callbackSenderThread = _callbackSenderThread;
        _callbackSenderThread = 0; // Resolve cyclic dependency.
    }

    callbackSenderThread->getThreadControl().join();
}

void
CallbackSenderI::addClient(const Identity& ident, const Current& current)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    cout << "adding client `" << _communicator->identityToString(ident) << "'"<< endl;

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
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
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
                catch(const Exception& ex)
                {
                    cerr << "removing client `" << _communicator->identityToString((*p)->ice_getIdentity()) << "':\n"
                         << ex << endl;

                    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
                    _clients.erase(*p);
                }
            }
        }
    }
}
