//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_COMMUNICATOR_I_H
#define ICE_COMMUNICATOR_I_H

#include <Ice/DynamicLibraryF.h>
#include <Ice/Initialize.h>
#include <Ice/Communicator.h>
#include <Ice/OutgoingAsync.h>

#include <functional>

namespace IceInternal
{

//
// Class for handling Ice::Communicator::begin_flushBatchRequests
//
class CommunicatorFlushBatchAsync : public OutgoingAsyncBase
{
public:

    virtual ~CommunicatorFlushBatchAsync();

    CommunicatorFlushBatchAsync(const InstancePtr&);

    void flushConnection(const Ice::ConnectionIPtr&, Ice::CompressBatch);
    void invoke(const std::string&, Ice::CompressBatch);

    std::shared_ptr<CommunicatorFlushBatchAsync> shared_from_this()
    {
        return std::static_pointer_cast<CommunicatorFlushBatchAsync>(OutgoingAsyncBase::shared_from_this());
    }

private:

    void check(bool);

    int _useCount;
};

}

namespace Ice
{

class CommunicatorI;
using CommunicatorIPtr = std::shared_ptr<CommunicatorI>;

class CommunicatorI : public Communicator, public std::enable_shared_from_this<CommunicatorI>
{
public:

    virtual void destroy() noexcept;
    virtual void shutdown() noexcept;
    virtual void waitForShutdown() noexcept;
    virtual bool isShutdown() const noexcept;

    virtual std::optional<ObjectPrx> stringToProxy(const std::string&) const;
    virtual std::string proxyToString(const std::optional<ObjectPrx>&) const;

    virtual std::optional<ObjectPrx> propertyToProxy(const std::string&) const;
    virtual PropertyDict proxyToProperty(const std::optional<ObjectPrx>&, const std::string&) const;

    virtual std::string identityToString(const Identity&) const;

    virtual ObjectAdapterPtr createObjectAdapter(const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithEndpoints(const std::string&, const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithRouter(const std::string&, const RouterPrx&);

    virtual ImplicitContextPtr getImplicitContext() const noexcept;

    virtual PropertiesPtr getProperties() const noexcept;
    virtual LoggerPtr getLogger() const noexcept;
    virtual Ice::Instrumentation::CommunicatorObserverPtr getObserver() const noexcept;

    virtual std::optional<RouterPrx> getDefaultRouter() const;
    virtual void setDefaultRouter(const std::optional<RouterPrx>&);

    virtual std::optional<LocatorPrx> getDefaultLocator() const;
    virtual void setDefaultLocator(const std::optional<LocatorPrx>&);

    virtual PluginManagerPtr getPluginManager() const;

    virtual ValueFactoryManagerPtr getValueFactoryManager() const noexcept;

#ifdef ICE_SWIFT
    virtual dispatch_queue_t getClientDispatchQueue() const;
    virtual dispatch_queue_t getServerDispatchQueue() const;
#endif

    virtual void postToClientThreadPool(::std::function<void()> call);

    virtual ::std::function<void()>
    flushBatchRequestsAsync(CompressBatch,
                            ::std::function<void(::std::exception_ptr)>,
                            ::std::function<void(bool)> = nullptr);

    virtual ObjectPrx createAdmin(const ObjectAdapterPtr&, const Identity&);
    virtual std::optional<ObjectPrx> getAdmin() const;
    virtual void addAdminFacet(const std::shared_ptr<Object>&, const std::string&);
    virtual std::shared_ptr<Object> removeAdminFacet(const std::string&);
    virtual std::shared_ptr<Object> findAdminFacet(const std::string&);
    virtual FacetMap findAllAdminFacets();

    virtual ~CommunicatorI();

private:

    static CommunicatorIPtr create(const InitializationData&);

    //
    // Certain initialization tasks need to be completed after the
    // constructor.
    //
    void finishSetup(int&, const char*[]);

    friend ICE_API CommunicatorPtr initialize(int&, const char*[], const InitializationData&, Int);
    friend ICE_API CommunicatorPtr initialize(StringSeq&, const InitializationData&, Int);
    friend ICE_API CommunicatorPtr initialize(const InitializationData&, Int);
    friend ICE_API ::IceInternal::InstancePtr IceInternal::getInstance(const ::Ice::CommunicatorPtr&);
    friend ICE_API ::IceUtil::TimerPtr IceInternal::getInstanceTimer(const ::Ice::CommunicatorPtr&);
    const ::IceInternal::InstancePtr _instance;

    //
    // We don't want the dynamic libraries to be unloaded until the
    // Communicator's destructor is invoked.
    //
    const ::IceInternal::DynamicLibraryListPtr _dynamicLibraryList;
};

}

#endif
