// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef INSTRUMENTATION_I_H
#define INSTRUMENTATION_I_H

#include <Ice/Instrumentation.h>

class ObserverI : public virtual Ice::Instrumentation::Observer, public IceUtil::Mutex
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
ICE_DEFINE_PTR(ObserverIPtr, ObserverI);

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
ICE_DEFINE_PTR(ConnectionObserverIPtr, ConnectionObserverI);

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
ICE_DEFINE_PTR(ThreadObserverIPtr, ThreadObserverI);

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
ICE_DEFINE_PTR(DispatchObserverIPtr, DispatchObserverI);

class ChildInvocationObserverI : public virtual Ice::Instrumentation::ChildInvocationObserver, public ObserverI
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
ICE_DEFINE_PTR(ChildInvocationObserverIPtr, ChildInvocationObserverI);

class RemoteObserverI : public Ice::Instrumentation::RemoteObserver, public ChildInvocationObserverI
{
};
ICE_DEFINE_PTR(RemoteObserverIPtr, RemoteObserverI);

class CollocatedObserverI : public Ice::Instrumentation::CollocatedObserver, public ChildInvocationObserverI
{
};
ICE_DEFINE_PTR(CollocatedObserverIPtr, CollocatedObserverI);

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
    getRemoteObserver(const Ice::ConnectionInfoPtr&, const Ice::EndpointPtr&, Ice::Int, Ice::Int)
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!remoteObserver)
        {
            remoteObserver = ICE_MAKE_SHARED(RemoteObserverI);
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
            collocatedObserver = ICE_MAKE_SHARED(CollocatedObserverI);
            collocatedObserver->reset();
        }
        return collocatedObserver;
    }

    Ice::Int userExceptionCount;
    Ice::Int retriedCount;

    RemoteObserverIPtr remoteObserver;
    CollocatedObserverIPtr collocatedObserver;
};
ICE_DEFINE_PTR(InvocationObserverIPtr, InvocationObserverI);

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
            connectionEstablishmentObserver = ICE_MAKE_SHARED(ObserverI);
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
            endpointLookupObserver = ICE_MAKE_SHARED(ObserverI);
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
            connectionObserver = ICE_MAKE_SHARED(ConnectionObserverI);
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
            threadObserver = ICE_MAKE_SHARED(ThreadObserverI);
            threadObserver->reset();
        }
        return threadObserver;
   }

    virtual Ice::Instrumentation::InvocationObserverPtr
    getInvocationObserver(const Ice::ObjectPrxPtr&, const std::string&, const Ice::Context&)
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!invocationObserver)
        {
            invocationObserver = ICE_MAKE_SHARED(InvocationObserverI);
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
            dispatchObserver = ICE_MAKE_SHARED(DispatchObserverI);
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

    ObserverIPtr connectionEstablishmentObserver;
    ObserverIPtr endpointLookupObserver;
    ConnectionObserverIPtr connectionObserver;
    ThreadObserverIPtr threadObserver;
    InvocationObserverIPtr invocationObserver;
    DispatchObserverIPtr dispatchObserver;
};
ICE_DEFINE_PTR(CommunicatorObserverIPtr, CommunicatorObserverI);

#endif
