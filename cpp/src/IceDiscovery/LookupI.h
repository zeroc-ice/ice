// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef LOOKUPI_H
#define LOOKUPI_H

#include <IceDiscovery/IceDiscovery.h>
#include <IceDiscovery/LocatorI.h>

#include <IceUtil/Timer.h>
#include <Ice/Properties.h>

namespace IceDiscovery
{

class LookupI;

#ifdef ICE_CPP11_MAPPING

template<class T> class Request : public IceUtil::TimerTask
{
public:

    Request(std::shared_ptr<LookupI> lookup, const T& id, int retryCount) :
        _lookup(lookup),
        _id(id),
        _nRetry(retryCount)
    {
    }

    T getId() const
    {
        return _id;
    }

    virtual bool retry()
    {
        return --_nRetry >= 0;
    }

    bool addCallback(std::function<void(const std::shared_ptr<::Ice::ObjectPrx>&)> cb)
    {
        _callbacks.push_back(cb);
        return _callbacks.size() == 1;
    }

    virtual void finished(const Ice::ObjectPrxPtr& proxy)
    {
        for(auto cb : _callbacks)
        {
            cb(proxy);
        }
        _callbacks.clear();
    }

protected:

    LookupIPtr _lookup;
    const T _id;
    int _nRetry;
    std::vector<std::function<void(const std::shared_ptr<::Ice::ObjectPrx>&)>> _callbacks;
};

class ObjectRequest : public Request<Ice::Identity>, public std::enable_shared_from_this<ObjectRequest>
{
public:

    ObjectRequest(const std::shared_ptr<LookupI>& lookup, const Ice::Identity& id, int retryCount) :
        Request<Ice::Identity>(lookup, id, retryCount)
    {
    }

    void response(const std::shared_ptr<Ice::ObjectPrx>&);

private:

    virtual void runTimerTask();
};
typedef std::shared_ptr<ObjectRequest> ObjectRequestPtr;

class AdapterRequest : public Request<std::string>, public std::enable_shared_from_this<AdapterRequest>
{
public:

    AdapterRequest(std::shared_ptr<LookupI> lookup, const std::string& adapterId, int retryCount) :
        Request<std::string>(lookup, adapterId, retryCount),
        _start(IceUtil::Time::now())
    {
    }

    bool response(const std::shared_ptr<Ice::ObjectPrx>&, bool);

    virtual bool retry();
    virtual void finished(const std::shared_ptr<Ice::ObjectPrx>&);

private:

    virtual void runTimerTask();
    std::vector<Ice::ObjectPrxPtr> _proxies;
    IceUtil::Time _start;
    IceUtil::Time _latency;
};
typedef std::shared_ptr<AdapterRequest> AdapterRequestPtr;

#else

class Request : public IceUtil::TimerTask
{
public:

    Request(const LookupIPtr&, int);

    virtual bool retry();

protected:

    LookupIPtr _lookup;
    int _nRetry;
};

template<class T, class CB> class RequestT : public Request
{
public:

    RequestT(LookupI* lookup, T id, int retryCount) : Request(lookup, retryCount), _id(id)
    {
    }

    T getId() const
    {
        return _id;
    }

    bool addCallback(CB cb)
    {
        _callbacks.push_back(cb);
        return _callbacks.size() == 1;
    }

    virtual void finished(const Ice::ObjectPrxPtr& proxy)
    {
        for(typename std::vector<CB>::const_iterator p = _callbacks.begin(); p != _callbacks.end(); ++p)
        {
            (*p)->ice_response(proxy);
        }
        _callbacks.clear();
    }

protected:

    const T _id;
    std::vector<CB> _callbacks;
};

class ObjectRequest : public RequestT<Ice::Identity, Ice::AMD_Locator_findObjectByIdPtr>
{
public:

    ObjectRequest(LookupI* lookup, const Ice::Identity& id, int retryCount) :
        RequestT<Ice::Identity, Ice::AMD_Locator_findObjectByIdPtr>(lookup, id, retryCount)
    {
    }

    void response(const Ice::ObjectPrxPtr&);

private:

    virtual void runTimerTask();
};
typedef IceUtil::Handle<ObjectRequest> ObjectRequestPtr;

class AdapterRequest : public RequestT<std::string, Ice::AMD_Locator_findAdapterByIdPtr>
{
public:

    AdapterRequest(LookupI* lookup, const std::string& adapterId, int retryCount) :
        RequestT<std::string, Ice::AMD_Locator_findAdapterByIdPtr>(lookup, adapterId, retryCount),
        _start(IceUtil::Time::now())
    {
    }

    bool response(const Ice::ObjectPrxPtr&, bool);

    virtual bool retry();
    virtual void finished(const Ice::ObjectPrxPtr&);

private:

    virtual void runTimerTask();
    std::vector<Ice::ObjectPrxPtr> _proxies;
    IceUtil::Time _start;
    IceUtil::Time _latency;
};
typedef IceUtil::Handle<AdapterRequest> AdapterRequestPtr;

#endif

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

#ifdef ICE_CPP11_MAPPING
    virtual void findObjectById(std::string,
                                Ice::Identity,
                                ::std::shared_ptr<IceDiscovery::LookupReplyPrx>,
                                const Ice::Current&);
    virtual void findAdapterById(std::string, std::string, ::std::shared_ptr<IceDiscovery::LookupReplyPrx>,
                                 const Ice::Current&);
    void findObject(std::function<void(const std::shared_ptr<Ice::ObjectPrx>&)>, const Ice::Identity&);
    void findAdapter(std::function<void(const std::shared_ptr<Ice::ObjectPrx>&)>, const std::string&);
#else
    virtual void findObjectById(const std::string&, const Ice::Identity&, const IceDiscovery::LookupReplyPrx&,
                                const Ice::Current&);
    virtual void findAdapterById(const std::string&, const std::string&, const IceDiscovery::LookupReplyPrx&,
                                 const Ice::Current&);
    void findObject(const Ice::AMD_Locator_findObjectByIdPtr&, const Ice::Identity&);
    void findAdapter(const Ice::AMD_Locator_findAdapterByIdPtr&, const std::string&);
#endif

    void foundObject(const Ice::Identity&, const Ice::ObjectPrxPtr&);
    void foundAdapter(const std::string&, const Ice::ObjectPrxPtr&, bool);

    void adapterRequestTimedOut(const AdapterRequestPtr&);
    void objectRequestTimedOut(const ObjectRequestPtr&);

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
    std::vector<std::pair<LookupPrxPtr, LookupReplyPrxPtr> > _lookup;
    const IceUtil::Time _timeout;
    const int _retryCount;
    const int _latencyMultiplier;
    const std::string _domainId;

    IceUtil::TimerPtr _timer;
    Ice::ObjectPrxPtr _wellKnownProxy;

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
