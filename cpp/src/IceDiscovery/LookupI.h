// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

    virtual void finished(const Ice::ObjectPrx& proxy)
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

    void response(const Ice::ObjectPrx&);

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

    bool response(const Ice::ObjectPrx&, bool);

    virtual bool retry();
    virtual void finished(const Ice::ObjectPrx&);

private:

    virtual void runTimerTask();
    std::vector<Ice::ObjectPrx> _proxies;
    IceUtil::Time _start;
    IceUtil::Time _latency;
};
typedef IceUtil::Handle<AdapterRequest> AdapterRequestPtr;

class LookupI : public Lookup, private IceUtil::Mutex
{
public:

    LookupI(const LocatorRegistryIPtr&, const LookupPrx&, const Ice::PropertiesPtr&);
    virtual ~LookupI();

    void destroy();

    void setLookupReply(const LookupReplyPrx&);

    virtual void findObjectById(const std::string&, const Ice::Identity&, const IceDiscovery::LookupReplyPrx&, 
                                const Ice::Current&);
    virtual void findAdapterById(const std::string&, const std::string&, const IceDiscovery::LookupReplyPrx&, 
                                 const Ice::Current&);

    void findObject(const Ice::AMD_Locator_findObjectByIdPtr&, const Ice::Identity&);
    void findAdapter(const Ice::AMD_Locator_findAdapterByIdPtr&, const std::string&);

    void foundObject(const Ice::Identity&, const Ice::ObjectPrx&);
    void foundAdapter(const std::string&, const Ice::ObjectPrx&, bool);

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
    const LookupPrx _lookup;
    LookupReplyPrx _lookupReply;
    const IceUtil::Time _timeout;
    const int _retryCount;
    const int _latencyMultiplier;
    const std::string _domainId;

    IceUtil::TimerPtr _timer;
    Ice::ObjectPrx _wellKnownProxy;

    std::map<Ice::Identity, ObjectRequestPtr> _objectRequests;
    std::map<std::string, AdapterRequestPtr> _adapterRequests;
};

class LookupReplyI : public LookupReply
{
public:

    LookupReplyI(const LookupIPtr&);

    virtual void foundObjectById(const Ice::Identity&, const Ice::ObjectPrx&, const Ice::Current&);
    virtual void foundAdapterById(const std::string&, const Ice::ObjectPrx&, bool, const Ice::Current&);

private:

    const LookupIPtr _lookup;
};

};

#endif
