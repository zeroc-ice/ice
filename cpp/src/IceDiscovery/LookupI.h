//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef LOOKUPI_H
#define LOOKUPI_H

#include <IceDiscovery/IceDiscovery.h>
#include <IceDiscovery/LocatorI.h>

#include <IceUtil/Timer.h>
#include <Ice/Properties.h>
#include <Ice/Comparable.h>

#include <set>
#include <chrono>

namespace IceDiscovery
{

class LookupI;

class Request : public IceUtil::TimerTask
{
public:

    Request(const LookupIPtr&, int);

    virtual bool retry();
    void invoke(const std::string&, const std::vector<std::pair<LookupPrx, LookupReplyPrx> >&);
    bool exception();
    std::string getRequestId() const;

    virtual void finished(const Ice::ObjectPrxPtr&) = 0;

protected:

    virtual void invokeWithLookup(const std::string&, const LookupPrx&, const LookupReplyPrx&) = 0;

    LookupIPtr _lookup;
    const std::string _requestId;
    int _retryCount;
    size_t _lookupCount;
    size_t _failureCount;
};
using RequestPtr = std::shared_ptr<Request>;

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
            p->first(proxy);
        }
        _callbacks.clear();
    }

protected:

    const T _id;
    std::vector<CB> _callbacks;
};

typedef std::pair<std::function<void(const ::Ice::ObjectPrxPtr&)>,
                  std::function<void(std::exception_ptr)>> ObjectCB;
typedef std::pair<std::function<void(const ::Ice::ObjectPrxPtr&)>,
                  std::function<void(std::exception_ptr)>> AdapterCB;

class ObjectRequest : public RequestT<Ice::Identity, ObjectCB>, public std::enable_shared_from_this<ObjectRequest>
{
public:

    ObjectRequest(const LookupIPtr&, const Ice::Identity&, int);

    void response(const Ice::ObjectPrxPtr&);

private:

    virtual void invokeWithLookup(const std::string&, const LookupPrx&, const LookupReplyPrx&);
    virtual void runTimerTask();
};
using ObjectRequestPtr = std::shared_ptr<ObjectRequest>;

class AdapterRequest : public RequestT<std::string, AdapterCB>, public std::enable_shared_from_this<AdapterRequest>
{
public:

    AdapterRequest(const LookupIPtr&, const std::string&, int);

    bool response(const Ice::ObjectPrxPtr&, bool);

    virtual bool retry();
    virtual void finished(const Ice::ObjectPrxPtr&);

private:

    virtual void invokeWithLookup(const std::string&, const LookupPrx&, const LookupReplyPrx&);
    virtual void runTimerTask();

    //
    // We use a set because the same IceDiscovery plugin might return multiple times
    // the same proxy if it's accessible through multiple network interfaces and if we
    // also sent the request to multiple interfaces.
    //
    std::set<Ice::ObjectPrxPtr> _proxies;
    std::chrono::steady_clock::time_point _start;
    std::chrono::nanoseconds _latency;
};
using AdapterRequestPtr = std::shared_ptr<AdapterRequest>;

class LookupI : public Lookup, public std::enable_shared_from_this<LookupI>
{
public:

    LookupI(const LocatorRegistryIPtr&, const LookupPrx&, const Ice::PropertiesPtr&);
    virtual ~LookupI();

    void destroy();

    void setLookupReply(const LookupReplyPrx&);

    virtual void findObjectById(std::string, Ice::Identity, IceDiscovery::LookupReplyPrxPtr,
                                const Ice::Current&);
    virtual void findAdapterById(std::string, std::string, IceDiscovery::LookupReplyPrxPtr,
                                 const Ice::Current&);
    void findObject(const ObjectCB&, const Ice::Identity&);
    void findAdapter(const AdapterCB&, const std::string&);

    void foundObject(const Ice::Identity&, const std::string&, const Ice::ObjectPrxPtr&);
    void foundAdapter(const std::string&, const std::string&, const Ice::ObjectPrxPtr&, bool);

    void adapterRequestTimedOut(const AdapterRequestPtr&);
    void adapterRequestException(const AdapterRequestPtr&, std::exception_ptr);
    void objectRequestTimedOut(const ObjectRequestPtr&);
    void objectRequestException(const ObjectRequestPtr&, std::exception_ptr);

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
    LookupPrx _lookup;
    std::vector<std::pair<LookupPrx, LookupReplyPrx>> _lookups;
    const std::chrono::milliseconds _timeout;
    const int _retryCount;
    const int _latencyMultiplier;
    const std::string _domainId;

    IceUtil::TimerPtr _timer;
    Ice::ObjectPrx _wellKnownProxy;
    bool _warnOnce;

    std::map<Ice::Identity, ObjectRequestPtr> _objectRequests;
    std::map<std::string, AdapterRequestPtr> _adapterRequests;
    std::mutex _mutex;
};

class LookupReplyI final : public LookupReply
{
public:

    LookupReplyI(const LookupIPtr&);

    void foundObjectById(Ice::Identity, Ice::ObjectPrxPtr, const Ice::Current&) final;
    void foundAdapterById(std::string, Ice::ObjectPrxPtr, bool, const Ice::Current&) final;

private:

    const LookupIPtr _lookup;
};

};

#endif
