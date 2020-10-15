//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_COMMUNICATOR_I_H
#define ICE_COMMUNICATOR_I_H

#include <IceUtil/RecMutex.h>

#include <Ice/DynamicLibraryF.h>
#include <Ice/Initialize.h>
#include <Ice/Communicator.h>
#include <Ice/OutgoingAsync.h>

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
ICE_DEFINE_PTR(CommunicatorIPtr, CommunicatorI);

class CommunicatorI : public Communicator
                    , public std::enable_shared_from_this<CommunicatorI>

{
public:

    virtual void destroy() noexcept;
    virtual void shutdown() noexcept;
    virtual void waitForShutdown() noexcept;
    virtual bool isShutdown() const noexcept;

    virtual ObjectPrxPtr stringToProxy(const std::string&) const;
    virtual std::string proxyToString(const ObjectPrxPtr&) const;

    virtual ObjectPrxPtr propertyToProxy(const std::string&) const;
    virtual PropertyDict proxyToProperty(const ObjectPrxPtr&, const std::string&) const;

    virtual Identity stringToIdentity(const std::string&) const;
    virtual std::string identityToString(const Identity&) const;

    virtual ObjectAdapterPtr createObjectAdapter(const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithEndpoints(const std::string&, const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithRouter(const std::string&, const RouterPrxPtr&);

    virtual void addObjectFactory(const ObjectFactoryPtr&, const std::string&);
    virtual ObjectFactoryPtr findObjectFactory(const std::string&) const noexcept;

    virtual ImplicitContextPtr getImplicitContext() const noexcept;

    virtual PropertiesPtr getProperties() const noexcept;
    virtual LoggerPtr getLogger() const noexcept;
    virtual Ice::Instrumentation::CommunicatorObserverPtr getObserver() const noexcept;

    virtual RouterPrxPtr getDefaultRouter() const;
    virtual void setDefaultRouter(const RouterPrxPtr&);

    virtual LocatorPrxPtr getDefaultLocator() const;
    virtual void setDefaultLocator(const LocatorPrxPtr&);

    virtual PluginManagerPtr getPluginManager() const;

    virtual ValueFactoryManagerPtr getValueFactoryManager() const noexcept;

#ifdef ICE_SWIFT
    virtual dispatch_queue_t getClientDispatchQueue() const;
    virtual dispatch_queue_t getServerDispatchQueue() const;
#endif

    virtual ::std::function<void()>
    flushBatchRequestsAsync(CompressBatch,
                            ::std::function<void(::std::exception_ptr)>,
                            ::std::function<void(bool)> = nullptr);

    virtual ObjectPrxPtr createAdmin(const ObjectAdapterPtr&, const Identity&);
    virtual ObjectPrxPtr getAdmin() const;
    virtual void addAdminFacet(const ObjectPtr&, const std::string&);
    virtual ObjectPtr removeAdminFacet(const std::string&);
    virtual ObjectPtr findAdminFacet(const std::string&);
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
