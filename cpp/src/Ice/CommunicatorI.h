//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_COMMUNICATOR_I_H
#define ICE_COMMUNICATOR_I_H

#include <IceUtil/RecMutex.h>

#include <Ice/DynamicLibraryF.h>
#include <Ice/Initialize.h>
#include <Ice/Communicator.h>
#include <Ice/CommunicatorAsync.h>
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

#ifdef ICE_CPP11_MAPPING
    std::shared_ptr<CommunicatorFlushBatchAsync> shared_from_this()
    {
        return std::static_pointer_cast<CommunicatorFlushBatchAsync>(OutgoingAsyncBase::shared_from_this());
    }
#endif

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
#ifdef ICE_CPP11_MAPPING
                    , public std::enable_shared_from_this<CommunicatorI>
#endif

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

    virtual std::string identityToString(const Identity&) const;

    virtual ObjectAdapterPtr createObjectAdapter(const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithEndpoints(const std::string&, const std::string&);
    virtual ObjectAdapterPtr createObjectAdapterWithRouter(const std::string&, const RouterPrxPtr&);

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

#ifdef ICE_CPP11_MAPPING
    virtual ::std::function<void()>
    flushBatchRequestsAsync(CompressBatch,
                            ::std::function<void(::std::exception_ptr)>,
                            ::std::function<void(bool)> = nullptr);
#else
    virtual void flushBatchRequests(CompressBatch);
    virtual AsyncResultPtr begin_flushBatchRequests(CompressBatch);
    virtual AsyncResultPtr begin_flushBatchRequests(CompressBatch, const CallbackPtr&, const LocalObjectPtr& = 0);
    virtual AsyncResultPtr begin_flushBatchRequests(CompressBatch,
                                                    const Callback_Communicator_flushBatchRequestsPtr&,
                                                    const LocalObjectPtr& = 0);

    virtual void end_flushBatchRequests(const AsyncResultPtr&);
#endif

    virtual ObjectPrxPtr createAdmin(const ObjectAdapterPtr&, const Identity&);
    virtual ObjectPrxPtr getAdmin() const;
    virtual void addAdminFacet(const std::shared_ptr<Object>&, const std::string&);
    virtual std::shared_ptr<Object> removeAdminFacet(const std::string&);
    virtual std::shared_ptr<Object> findAdminFacet(const std::string&);
    virtual FacetMap findAllAdminFacets();

    virtual ~CommunicatorI();

private:

#ifndef ICE_CPP11_MAPPING
    CommunicatorI() {}
#endif

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

#ifndef ICE_CPP11_MAPPING
    AsyncResultPtr _iceI_begin_flushBatchRequests(CompressBatch,
                                                  const IceInternal::CallbackBasePtr&,
                                                  const LocalObjectPtr&);
#endif

    const ::IceInternal::InstancePtr _instance;

    //
    // We don't want the dynamic libraries to be unloaded until the
    // Communicator's destructor is invoked.
    //
    const ::IceInternal::DynamicLibraryListPtr _dynamicLibraryList;
};

}

#endif
