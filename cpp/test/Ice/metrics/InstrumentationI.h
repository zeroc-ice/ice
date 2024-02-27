//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef INSTRUMENTATION_I_H
#define INSTRUMENTATION_I_H

#include <Ice/Instrumentation.h>
#include <mutex>

class ObserverI : public virtual Ice::Instrumentation::Observer
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
        std::lock_guard lock(_mutex);
        ++total;
        ++current;
    }

    virtual void
    detach()
    {
        std::lock_guard lock(_mutex);
        --current;
    }

    virtual void
    failed(const std::string&)
    {
        std::lock_guard lock(_mutex);
        ++failedCount;
    }

    std::int32_t
    getTotal() const
    {
        std::lock_guard lock(_mutex);
        return total;
    }

    std::int32_t
    getCurrent() const
    {
        std::lock_guard lock(_mutex);
        return current;
    }

    std::int32_t
    getFailedCount() const
    {
        std::lock_guard lock(_mutex);
        return failedCount;
    }

    std::int32_t total;
    std::int32_t current;
    std::int32_t failedCount;

protected:

    mutable std::mutex _mutex;
};
using ObserverIPtr = std::shared_ptr<ObserverI>;

class ConnectionObserverI : public Ice::Instrumentation::ConnectionObserver, public ObserverI
{
public:

    virtual void
    reset()
    {
        std::lock_guard lock(_mutex);
        ObserverI::reset();
        received = 0;
        sent = 0;
    }

    virtual void
    sentBytes(std::int32_t s)
    {
        std::lock_guard lock(_mutex);
        sent += s;
    }

    virtual void
    receivedBytes(std::int32_t s)
    {
        std::lock_guard lock(_mutex);
        received += s;
    }

    std::int32_t sent;
    std::int32_t received;
};
using ConnectionObserverIPtr = std::shared_ptr<ConnectionObserverI>;

class ThreadObserverI : public Ice::Instrumentation::ThreadObserver, public ObserverI
{
public:

    virtual void
    reset()
    {
        std::lock_guard lock(_mutex);
        ObserverI::reset();
        states = 0;
    }

    virtual void
    stateChanged(Ice::Instrumentation::ThreadState, Ice::Instrumentation::ThreadState)
    {
        std::lock_guard lock(_mutex);
        ++states;
    }

    std::int32_t states;
};
using ThreadObserverIPtr = std::shared_ptr<ThreadObserverI>;

class DispatchObserverI : public Ice::Instrumentation::DispatchObserver, public ObserverI
{
public:

    virtual void reset()
    {
        std::lock_guard lock(_mutex);
        ObserverI::reset();
        userExceptionCount = 0;
        replySize = 0;
    }

    virtual void
    userException()
    {
        std::lock_guard lock(_mutex);
        ++userExceptionCount;
    }

    virtual void
    reply(std::int32_t s)
    {
        std::lock_guard lock(_mutex);
        replySize += s;
    }

    std::int32_t userExceptionCount;
    std::int32_t replySize;
};
using DispatchObserverIPtr = std::shared_ptr<DispatchObserverI>;

class ChildInvocationObserverI : public virtual Ice::Instrumentation::ChildInvocationObserver, public ObserverI
{
public:

    virtual void
    reset()
    {
        std::lock_guard lock(_mutex);
        ObserverI::reset();
        replySize = 0;
    }

    virtual void
    reply(std::int32_t s)
    {
        std::lock_guard lock(_mutex);
        replySize += s;
    }

    std::int32_t replySize;
};
using ChildInvocationObserverIPtr = std::shared_ptr<ChildInvocationObserverI>;

class RemoteObserverI : public Ice::Instrumentation::RemoteObserver, public ChildInvocationObserverI
{
};
using RemoteObserverIPtr = std::shared_ptr<RemoteObserverI>;

class CollocatedObserverI : public Ice::Instrumentation::CollocatedObserver, public ChildInvocationObserverI
{
};
using CollocatedObserverIPtr = std::shared_ptr<CollocatedObserverI>;

class InvocationObserverI : public Ice::Instrumentation::InvocationObserver, public ObserverI
{
public:

    virtual void reset()
    {
        std::lock_guard lock(_mutex);
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
        std::lock_guard lock(_mutex);
        ++retriedCount;
    }

    virtual void
    userException()
    {
        std::lock_guard lock(_mutex);
        ++userExceptionCount;
    }

    virtual Ice::Instrumentation::RemoteObserverPtr
    getRemoteObserver(const Ice::ConnectionInfoPtr&, const Ice::EndpointPtr&, std::int32_t, std::int32_t)
    {
        std::lock_guard lock(_mutex);
        if(!remoteObserver)
        {
            remoteObserver = std::make_shared<RemoteObserverI>();
            remoteObserver->reset();
        }
        return remoteObserver;
    }

    virtual Ice::Instrumentation::CollocatedObserverPtr
    getCollocatedObserver(const Ice::ObjectAdapterPtr&, std::int32_t, std::int32_t)
    {
        std::lock_guard lock(_mutex);
        if(!collocatedObserver)
        {
            collocatedObserver = std::make_shared<CollocatedObserverI>();
            collocatedObserver->reset();
        }
        return collocatedObserver;
    }

    std::int32_t userExceptionCount;
    std::int32_t retriedCount;

    RemoteObserverIPtr remoteObserver;
    CollocatedObserverIPtr collocatedObserver;
};
using InvocationObserverIPtr = std::shared_ptr<InvocationObserverI>;

class CommunicatorObserverI : public Ice::Instrumentation::CommunicatorObserver
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
        std::lock_guard lock(_mutex);
        if(!connectionEstablishmentObserver)
        {
            connectionEstablishmentObserver = std::make_shared<ObserverI>();
            connectionEstablishmentObserver->reset();
        }
        return connectionEstablishmentObserver;
    }

    virtual Ice::Instrumentation::ObserverPtr
    getEndpointLookupObserver(const Ice::EndpointPtr&)
    {
        std::lock_guard lock(_mutex);
        if(!endpointLookupObserver)
        {
            endpointLookupObserver = std::make_shared<ObserverI>();
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
        std::lock_guard lock(_mutex);
        test(!old || dynamic_cast<ConnectionObserverI*>(old.get()));
        if(!connectionObserver)
        {
            connectionObserver = std::make_shared<ConnectionObserverI>();
            connectionObserver->reset();
        }
        return connectionObserver;
    }

    virtual Ice::Instrumentation::ThreadObserverPtr
    getThreadObserver(const std::string&, const std::string&, Ice::Instrumentation::ThreadState,
                      const Ice::Instrumentation::ThreadObserverPtr& old)
    {
        std::lock_guard lock(_mutex);
        test(!old || dynamic_cast<ThreadObserverI*>(old.get()));
        if(!threadObserver)
        {
            threadObserver = std::make_shared<ThreadObserverI>();
            threadObserver->reset();
        }
        return threadObserver;
   }

    virtual Ice::Instrumentation::InvocationObserverPtr
    getInvocationObserver(const Ice::ObjectPrxPtr&, const std::string&, const Ice::Context&)
    {
        std::lock_guard lock(_mutex);
        if(!invocationObserver)
        {
            invocationObserver = std::make_shared<InvocationObserverI>();
            invocationObserver->reset();
        }
        return invocationObserver;
    }

    virtual Ice::Instrumentation::DispatchObserverPtr
    getDispatchObserver(const Ice::Current&, std::int32_t)
    {
        std::lock_guard lock(_mutex);
        if(!dispatchObserver)
        {
            dispatchObserver = std::make_shared<DispatchObserverI>();
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

private:

    std::mutex _mutex;
};
using CommunicatorObserverIPtr = std::shared_ptr<CommunicatorObserverI>;

#endif
