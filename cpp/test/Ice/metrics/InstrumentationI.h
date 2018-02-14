// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef INSTRUMENTATION_I_H
#define INSTRUMENTATION_I_H

#include <Ice/Instrumentation.h>

class ObserverI : virtual public Ice::Instrumentation::Observer, public IceUtil::Mutex
{
public:

    virtual void 
    reset()
    {
        total = 0;
        current = 0;
        failedCount = 0;
    }

    virtual void 
    attach()
    {
        IceUtil::Mutex::Lock sync(*this);
        ++total;
        ++current;
    }

    virtual void 
    detach()
    {
        IceUtil::Mutex::Lock sync(*this);
        --current;
    }

    virtual void 
    failed(const std::string&)
    {
        IceUtil::Mutex::Lock sync(*this);
        ++failedCount;
    }

    Ice::Int
    getTotal() const
    {
        IceUtil::Mutex::Lock sync(*this);
        return total;
    }

    Ice::Int
    getCurrent() const
    {
        IceUtil::Mutex::Lock sync(*this);
        return current;
    }

    Ice::Int
    getFailedCount() const
    {
        IceUtil::Mutex::Lock sync(*this);
        return failedCount;
    }

    Ice::Int total;
    Ice::Int current;
    Ice::Int failedCount;
};

class ConnectionObserverI : public Ice::Instrumentation::ConnectionObserver, public ObserverI
{
public:

    virtual void 
    reset()
    {
        IceUtil::Mutex::Lock sync(*this);
        ObserverI::reset();
        received = 0;
        sent = 0;
    }

    virtual void 
    sentBytes(Ice::Int s)
    {
        IceUtil::Mutex::Lock sync(*this);
        sent += s;
    }

    virtual void 
    receivedBytes(Ice::Int s)
    {
        IceUtil::Mutex::Lock sync(*this);
        received += s;
    }
    
    Ice::Int sent;
    Ice::Int received;
};

class ThreadObserverI : public Ice::Instrumentation::ThreadObserver, public ObserverI
{
public:

    virtual void 
    reset()
    {
        IceUtil::Mutex::Lock sync(*this);
        ObserverI::reset();
        states = 0;
    }

    virtual void 
    stateChanged(Ice::Instrumentation::ThreadState, Ice::Instrumentation::ThreadState)
    {
        IceUtil::Mutex::Lock sync(*this);
        ++states;
    }

    Ice::Int states;
};

class DispatchObserverI : public Ice::Instrumentation::DispatchObserver, public ObserverI
{
public:

    virtual void reset()
    {
        IceUtil::Mutex::Lock sync(*this);
        ObserverI::reset();
        userExceptionCount = 0;
        replySize = 0;
    }

    virtual void 
    userException()
    {
        IceUtil::Mutex::Lock sync(*this);
        ++userExceptionCount;
    }

    virtual void 
    reply(Ice::Int s)
    {
        IceUtil::Mutex::Lock sync(*this);
        replySize += s;
    }

    Ice::Int userExceptionCount;
    Ice::Int replySize;
};

class ChildInvocationObserverI : virtual public Ice::Instrumentation::ChildInvocationObserver, public ObserverI
{
public:

    virtual void 
    reset()
    {
        IceUtil::Mutex::Lock sync(*this);
        ObserverI::reset();
        replySize = 0;
    }

    virtual void
    reply(Ice::Int s)
    {
        IceUtil::Mutex::Lock sync(*this);
        replySize += s;
    }

    Ice::Int replySize;
};

class RemoteObserverI : public Ice::Instrumentation::RemoteObserver, public ChildInvocationObserverI
{
};

class CollocatedObserverI : public Ice::Instrumentation::CollocatedObserver, public ChildInvocationObserverI
{
};

class InvocationObserverI : public Ice::Instrumentation::InvocationObserver, public ObserverI
{
public:

    virtual void reset()
    {
        IceUtil::Mutex::Lock sync(*this);
        ObserverI::reset();
        retriedCount = 0;
        userExceptionCount = 0;
        if(collocatedObserver)
        {
            collocatedObserver->reset();
        }
        if(remoteObserver)
        {
            remoteObserver->reset();
        }
    }

    virtual void 
    retried()
    {
        IceUtil::Mutex::Lock sync(*this);
        ++retriedCount;
    }

    virtual void 
    userException()
    {
        IceUtil::Mutex::Lock sync(*this);
        ++userExceptionCount;
    }

    virtual Ice::Instrumentation::RemoteObserverPtr 
    getRemoteObserver(const Ice::ConnectionInfoPtr& c, const Ice::EndpointPtr& e, Ice::Int, Ice::Int)
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!remoteObserver)
        {
            remoteObserver = new RemoteObserverI();
            remoteObserver->reset();
        }
        return remoteObserver;
    }

    virtual Ice::Instrumentation::CollocatedObserverPtr 
    getCollocatedObserver(const Ice::ObjectAdapterPtr&, Ice::Int, Ice::Int)
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!collocatedObserver)
        {
            collocatedObserver = new CollocatedObserverI();
            collocatedObserver->reset();
        }
        return collocatedObserver;
    }

    Ice::Int userExceptionCount;
    Ice::Int retriedCount;

    IceUtil::Handle<RemoteObserverI> remoteObserver;
    IceUtil::Handle<CollocatedObserverI> collocatedObserver;
};

class CommunicatorObserverI : public Ice::Instrumentation::CommunicatorObserver, public IceUtil::Mutex
{
public:

    virtual void 
    setObserverUpdater(const Ice::Instrumentation::ObserverUpdaterPtr& u)
    {
        updater = u;
    }
 
    virtual Ice::Instrumentation::ObserverPtr 
    getConnectionEstablishmentObserver(const Ice::EndpointPtr&, const std::string&)
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!connectionEstablishmentObserver)
        {
            connectionEstablishmentObserver = new ObserverI();
            connectionEstablishmentObserver->reset();
        }
        return connectionEstablishmentObserver;
    }

 
    virtual Ice::Instrumentation::ObserverPtr 
    getEndpointLookupObserver(const Ice::EndpointPtr&)
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!endpointLookupObserver)
        {
            endpointLookupObserver = new ObserverI();
            endpointLookupObserver->reset();
        }
        return endpointLookupObserver;
    }
    
    virtual Ice::Instrumentation::ConnectionObserverPtr 
    getConnectionObserver(const Ice::ConnectionInfoPtr&, 
                          const Ice::EndpointPtr&,
                          Ice::Instrumentation::ConnectionState, 
                          const Ice::Instrumentation::ConnectionObserverPtr& old)
    {
        IceUtil::Mutex::Lock sync(*this);
        test(!old || dynamic_cast<ConnectionObserverI*>(old.get()));
        if(!connectionObserver)
        {
            connectionObserver = new ConnectionObserverI();
            connectionObserver->reset();
        }
        return connectionObserver;
    }

    virtual Ice::Instrumentation::ThreadObserverPtr 
    getThreadObserver(const std::string&, const std::string&, Ice::Instrumentation::ThreadState,
                      const Ice::Instrumentation::ThreadObserverPtr& old)
    {
        IceUtil::Mutex::Lock sync(*this);
        test(!old || dynamic_cast<ThreadObserverI*>(old.get()));
        if(!threadObserver)
        {
            threadObserver = new ThreadObserverI();
            threadObserver->reset();
        }
        return threadObserver; 
   }

    virtual Ice::Instrumentation::InvocationObserverPtr 
    getInvocationObserver(const Ice::ObjectPrx&, const std::string&, const Ice::Context&)
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!invocationObserver)
        {
            invocationObserver = new InvocationObserverI();
            invocationObserver->reset();
        }
        return invocationObserver;
    }

    virtual Ice::Instrumentation::DispatchObserverPtr 
    getDispatchObserver(const Ice::Current&, Ice::Int)
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!dispatchObserver)
        {
            dispatchObserver = new DispatchObserverI();
            dispatchObserver->reset();
        }
        return dispatchObserver;
    }

    void reset()
    {
        if(connectionEstablishmentObserver)
        {
            connectionEstablishmentObserver->reset();
        }
        if(endpointLookupObserver)
        {
            endpointLookupObserver->reset();
        }
        if(connectionObserver)
        {
            connectionObserver->reset();
        }
        if(threadObserver)
        {
            threadObserver->reset();
        }
        if(invocationObserver)
        {
            invocationObserver->reset();
        }
        if(dispatchObserver)
        {
            dispatchObserver->reset();
        }
    }
    
    Ice::Instrumentation::ObserverUpdaterPtr updater;

    IceUtil::Handle<ObserverI> connectionEstablishmentObserver;
    IceUtil::Handle<ObserverI> endpointLookupObserver;
    IceUtil::Handle<ConnectionObserverI> connectionObserver;
    IceUtil::Handle<ThreadObserverI> threadObserver;
    IceUtil::Handle<InvocationObserverI> invocationObserver;
    IceUtil::Handle<DispatchObserverI> dispatchObserver;
};

typedef IceUtil::Handle<CommunicatorObserverI> CommunicatorObserverIPtr;

#endif
