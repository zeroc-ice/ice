// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef LOOKUPI_H
#define LOOKUPI_H

#include <IceDiscovery/IceDiscovery.h>
#include <IceDiscovery/LocatorI.h>

#include <IceUtil/Timer.h>
#include <Ice/Properties.h>
#include <Ice/Comparable.h>

#include <set>

namespace IceDiscovery
{

class LookupI;

class Request : public IceUtil::TimerTask
{
public:

    Request(const LookupIPtr&, int);

    virtual bool retry();
    void invoke(const std::string&, const std::vector<std::pair<LookupPrxPtr, LookupReplyPrxPtr> >&);
    bool exception();
    std::string getRequestId() const;

    virtual void finished(const Ice::ObjectPrxPtr&) = 0;

protected:

    virtual void invokeWithLookup(const std::string&, const LookupPrxPtr&, const LookupReplyPrxPtr&) = 0;

    LookupIPtr _lookup;
    const std::string _requestId;
    int _retryCount;
    size_t _lookupCount;
    size_t _failureCount;
};
ICE_DEFINE_PTR(RequestPtr, Request);

template<class T, class CB> class RequestT : public Request
{
public:

    RequestT(const LookupIPtr& lookup, T id, int retryCount) : Request(lookup, retryCount), _id(id)
    {
    }

    T getId() const
    {
        return _id;
    }

    bool addCallback(const CB& cb)
    {
        _callbacks.push_back(cb);
        return _callbacks.size() == 1;
    }

    virtual void finished(const Ice::ObjectPrxPtr& proxy)
    {
        for(typename std::vector<CB>::const_iterator p = _callbacks.begin(); p != _callbacks.end(); ++p)
        {
#ifdef ICE_CPP11_MAPPING
            p->first(proxy);
#else
            (*p)->ice_response(proxy);
#endif
        }
        _callbacks.clear();
    }

protected:

    const T _id;
    std::vector<CB> _callbacks;
};

#ifdef ICE_CPP11_MAPPING
typedef std::pair<std::function<void(const std::shared_ptr<::Ice::ObjectPrx>&)>,
                  std::function<void(std::exception_ptr)>> ObjectCB;
typedef std::pair<std::function<void(const std::shared_ptr<::Ice::ObjectPrx>&)>,
                  std::function<void(std::exception_ptr)>> AdapterCB;
#else
typedef Ice::AMD_Locator_findObjectByIdPtr ObjectCB;
typedef Ice::AMD_Locator_findAdapterByIdPtr AdapterCB;
#endif

class ObjectRequest : public RequestT<Ice::Identity, ObjectCB>
#ifdef ICE_CPP11_MAPPING
                      , public std::enable_shared_from_this<ObjectRequest>
#endif
{
public:

    ObjectRequest(const LookupIPtr&, const Ice::Identity&, int);

    void response(const Ice::ObjectPrxPtr&);

private:

    virtual void invokeWithLookup(const std::string&, const LookupPrxPtr&, const LookupReplyPrxPtr&);
    virtual void runTimerTask();
};
ICE_DEFINE_PTR(ObjectRequestPtr, ObjectRequest);

class AdapterRequest : public RequestT<std::string, AdapterCB>
#ifdef ICE_CPP11_MAPPING
                      , public std::enable_shared_from_this<AdapterRequest>
#endif
{
public:

    AdapterRequest(const LookupIPtr&, const std::string&, int);

    bool response(const Ice::ObjectPrxPtr&, bool);

    virtual bool retry();
    virtual void finished(const Ice::ObjectPrxPtr&);

private:

    virtual void invokeWithLookup(const std::string&, const LookupPrxPtr&, const LookupReplyPrxPtr&);
    virtual void runTimerTask();

    //
    // We use a set because the same IceDiscovery plugin might return multiple times
    // the same proxy if it's accessible through multiple network interfaces and if we
    // also sent the request to multiple interfaces.
    //
#ifdef ICE_CPP11_MAPPING
    std::set<std::shared_ptr<Ice::ObjectPrx>, Ice::TargetCompare<std::shared_ptr<Ice::ObjectPrx>, std::less>> _proxies;
#else
    std::set<Ice::ObjectPrx> _proxies;
#endif
    IceUtil::Time _start;
    IceUtil::Time _latency;
};
ICE_DEFINE_PTR(AdapterRequestPtr, AdapterRequest);

class LookupI : public Lookup,
                private IceUtil::Mutex
#ifdef ICE_CPP11_MAPPING
              , public std::enable_shared_from_this<LookupI>
#endif
{
public:

    LookupI(const LocatorRegistryIPtr&, const LookupPrxPtr&, const Ice::PropertiesPtr&);
    virtual ~LookupI();

    void destroy();

    void setLookupReply(const LookupReplyPrxPtr&);

    virtual void findObjectById(ICE_IN(std::string), ICE_IN(Ice::Identity), ICE_IN(IceDiscovery::LookupReplyPrxPtr),
                                const Ice::Current&);
    virtual void findAdapterById(ICE_IN(std::string), ICE_IN(std::string), ICE_IN(IceDiscovery::LookupReplyPrxPtr),
                                 const Ice::Current&);
    void findObject(const ObjectCB&, const Ice::Identity&);
    void findAdapter(const AdapterCB&, const std::string&);

    void foundObject(const Ice::Identity&, const std::string&, const Ice::ObjectPrxPtr&);
    void foundAdapter(const std::string&, const std::string&, const Ice::ObjectPrxPtr&, bool);

    void adapterRequestTimedOut(const AdapterRequestPtr&);
    void adapterRequestException(const AdapterRequestPtr&, const Ice::LocalException&);
    void objectRequestTimedOut(const ObjectRequestPtr&);
    void objectRequestException(const ObjectRequestPtr&, const Ice::LocalException&);

    const IceUtil::TimerPtr&
    timer()
    {
        return _timer;
    }

    int
    latencyMultiplier()
    {
        return _latencyMultiplier;
    }

private:

    LocatorRegistryIPtr _registry;
    LookupPrxPtr _lookup;
    std::vector<std::pair<LookupPrxPtr, LookupReplyPrxPtr> > _lookups;
    const IceUtil::Time _timeout;
    const int _retryCount;
    const int _latencyMultiplier;
    const std::string _domainId;

    IceUtil::TimerPtr _timer;
    Ice::ObjectPrxPtr _wellKnownProxy;
    bool _warnOnce;

    std::map<Ice::Identity, ObjectRequestPtr> _objectRequests;
    std::map<std::string, AdapterRequestPtr> _adapterRequests;
};

class LookupReplyI : public LookupReply
{
public:

    LookupReplyI(const LookupIPtr&);

#ifdef ICE_CPP11_MAPPING
    virtual void foundObjectById(Ice::Identity, std::shared_ptr<Ice::ObjectPrx>, const Ice::Current&);
    virtual void foundAdapterById(std::string, std::shared_ptr<Ice::ObjectPrx>, bool, const Ice::Current&);
#else
    virtual void foundObjectById(const Ice::Identity&, const Ice::ObjectPrx&, const Ice::Current&);
    virtual void foundAdapterById(const std::string&, const Ice::ObjectPrx&, bool, const Ice::Current&);
#endif

private:

    const LookupIPtr _lookup;
};

};

#endif
