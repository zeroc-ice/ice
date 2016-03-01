// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_LOCATOR_INFO_H
#define ICE_LOCATOR_INFO_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Time.h>
#include <Ice/LocatorInfoF.h>
#include <Ice/LocatorF.h>
#include <Ice/ReferenceF.h>
#include <Ice/Identity.h>
#include <Ice/EndpointIF.h>
#include <Ice/PropertiesF.h>
#include <Ice/Version.h>

#include <IceUtil/UniquePtr.h>

namespace IceInternal
{

class LocatorManager : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    LocatorManager(const Ice::PropertiesPtr&);

    void destroy();

    //
    // Returns locator info for a given locator. Automatically creates
    // the locator info if it doesn't exist yet.
    //
    LocatorInfoPtr get(const Ice::LocatorPrx&);

private:

    const bool _background;

    std::map<Ice::LocatorPrx, LocatorInfoPtr> _table;
    std::map<Ice::LocatorPrx, LocatorInfoPtr>::iterator _tableHint;

    std::map<std::pair<Ice::Identity, Ice::EncodingVersion>, LocatorTablePtr> _locatorTables;
};

class LocatorTable : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    LocatorTable();

    void clear();

    bool getAdapterEndpoints(const std::string&, int, ::std::vector<EndpointIPtr>&);
    void addAdapterEndpoints(const std::string&, const ::std::vector<EndpointIPtr>&);
    ::std::vector<EndpointIPtr> removeAdapterEndpoints(const std::string&);

    bool getObjectReference(const Ice::Identity&, int, ReferencePtr&);
    void addObjectReference(const Ice::Identity&, const ReferencePtr&);
    ReferencePtr removeObjectReference(const Ice::Identity&);

private:

    bool checkTTL(const IceUtil::Time&, int) const;

    std::map<std::string, std::pair<IceUtil::Time, std::vector<EndpointIPtr> > > _adapterEndpointsMap;
    std::map<Ice::Identity, std::pair<IceUtil::Time, ReferencePtr> > _objectMap;
};

class LocatorInfo : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    class GetEndpointsCallback : virtual public IceUtil::Shared
    {
    public:

        virtual void setEndpoints(const std::vector<EndpointIPtr>&, bool) = 0;
        virtual void setException(const Ice::LocalException&) = 0;
    };
    typedef IceUtil::Handle<GetEndpointsCallback> GetEndpointsCallbackPtr;

    class RequestCallback : virtual public IceUtil::Shared
    {
    public:

        RequestCallback(const ReferencePtr&, int, const GetEndpointsCallbackPtr&);

        void response(const LocatorInfoPtr&, const Ice::ObjectPrx&);
        void exception(const LocatorInfoPtr&, const Ice::Exception&);

    private:

        const ReferencePtr _ref;
        const int _ttl;
        const GetEndpointsCallbackPtr _callback;
    };
    typedef IceUtil::Handle<RequestCallback> RequestCallbackPtr;

    class Request : virtual public IceUtil::Shared
    {
    public:

        void addCallback(const ReferencePtr&, const ReferencePtr&, int, const GetEndpointsCallbackPtr&);
        std::vector<EndpointIPtr> getEndpoints(const ReferencePtr&, const ReferencePtr&, int, bool&);

        void response(const Ice::ObjectPrx&);
        void exception(const Ice::Exception&);

    protected:

        Request(const LocatorInfoPtr&, const ReferencePtr&);

        virtual void send() = 0;

        const LocatorInfoPtr _locatorInfo;
        const ReferencePtr _ref;

    private:

        IceUtil::Monitor<IceUtil::Mutex> _monitor;
        std::vector<RequestCallbackPtr> _callbacks;
        std::vector<ReferencePtr> _wellKnownRefs;
        bool _sent;
        bool _response;
        Ice::ObjectPrx _proxy;
        IceUtil::UniquePtr<Ice::Exception> _exception;
    };
    typedef IceUtil::Handle<Request> RequestPtr;

    LocatorInfo(const Ice::LocatorPrx&, const LocatorTablePtr&, bool);

    void destroy();

    bool operator==(const LocatorInfo&) const;
    bool operator!=(const LocatorInfo&) const;
    bool operator<(const LocatorInfo&) const;

    const Ice::LocatorPrx& getLocator() const
    {
        //
        // No mutex lock necessary, _locator is immutable.
        //
        return _locator;
    }
    Ice::LocatorRegistryPrx getLocatorRegistry();

    std::vector<EndpointIPtr> getEndpoints(const ReferencePtr& ref, int ttl, bool& cached)
    {
        return getEndpoints(ref, 0, ttl, cached);
    }
    std::vector<EndpointIPtr> getEndpoints(const ReferencePtr&, const ReferencePtr&, int, bool&);

    void getEndpointsWithCallback(const ReferencePtr& ref, int ttl, const GetEndpointsCallbackPtr& cb)
    {
        getEndpointsWithCallback(ref, 0, ttl, cb);
    }
    void getEndpointsWithCallback(const ReferencePtr&, const ReferencePtr&, int, const GetEndpointsCallbackPtr&);

    void clearCache(const ReferencePtr&);

private:

    void getEndpointsException(const ReferencePtr&, const Ice::Exception&);
    void getEndpointsTrace(const ReferencePtr&, const std::vector<EndpointIPtr>&, bool);
    void trace(const std::string&, const ReferencePtr&, const std::vector<EndpointIPtr>&);

    RequestPtr getAdapterRequest(const ReferencePtr&);
    RequestPtr getObjectRequest(const ReferencePtr&);

    void finishRequest(const ReferencePtr&, const std::vector<ReferencePtr>&, const Ice::ObjectPrx&, bool);
    friend class Request;
    friend class RequestCallback;

    const Ice::LocatorPrx _locator;
    Ice::LocatorRegistryPrx _locatorRegistry;
    const LocatorTablePtr _table;
    const bool _background;

    std::map<std::string, RequestPtr> _adapterRequests;
    std::map<Ice::Identity, RequestPtr> _objectRequests;
};

}

#endif
