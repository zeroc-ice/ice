// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_METRICSSTATS_I_H
#define ICE_METRICSSTATS_I_H

#include <Ice/Observer.h>
#include <Ice/Metrics.h>

#include <Ice/MetricsFunctional.h>

namespace IceMetrics
{

class MetricsAdminI;
typedef IceUtil::Handle<MetricsAdminI> MetricsAdminIPtr;

template<class T> class ObjectObserverI : virtual public Ice::ObjectObserver
{
public:

    typedef T Type;
    typedef IceUtil::Handle<T> PtrType;
    typedef std::vector<PtrType> SeqType;

    ObjectObserverI(const std::vector<MetricsObjectPtr>& objects)
    {
        for(std::vector<MetricsObjectPtr>::const_iterator p = objects.begin(); p != objects.end(); ++p)
        {
            _objects.push_back(PtrType::dynamicCast(*p));
        }
    }

    virtual void 
    attach()
    {
        for(typename SeqType::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            ++(*p)->total;
            ++(*p)->current;
        }
    }

    virtual void 
    detach()
    {
        forEach(decMember(&MetricsObject::current));
    }

    template<typename Function> void
    forEach(Function func)
    {
        for(typename SeqType::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            func(*p);
        }
    }

    void
    update(ObjectObserverI* old)
    {
        typename SeqType::const_iterator q = old->_objects.begin();
        for(typename SeqType::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
        {
            if(*p != *q)
            {
                ++(*p)->total;
                ++(*p)->current;
            }
            else
            {
                ++q;
            }
        }
    }

private:

    SeqType _objects;
};

class ConnectionObserverI : public Ice::ConnectionObserver, public ObjectObserverI<ConnectionMetricsObject>
{
public:

    ConnectionObserverI(const std::vector<MetricsObjectPtr>& objects) : ObjectObserverI(objects)
    {
    }

    virtual void attach();
    virtual void detach();

    virtual void stateChanged(Ice::ConnectionState, Ice::ConnectionState);
    virtual void sentBytes(Ice::Int, Ice::Long);
    virtual void receivedBytes(Ice::Int, Ice::Long);
};

template<typename T> 
class ObjectObserverResolver
{
public:

    typedef IceUtil::Handle<T> TPtr;
    typedef std::map<std::vector<MetricsObjectPtr>, TPtr> ObserverMap;

    template<typename S> T*
    getObserver(const std::vector<MetricsMap::Entry>& objects, S* oldObserver)
    {
        if(objects.empty())
        {
            return 0;
        }

        typename ObserverMap::const_iterator p = _stats.find(objects);
        if(p == _stats.end())
        {
            p = _stats.insert(make_pair(objects, new T(objects))).first;
        }
        
        T* newObserver = p->second.get();
        if(oldObserver && static_cast<S*>(newObserver) != oldObserver)
        {
            newObserver->update(dynamic_cast<T*>(oldObserver));
        }
        return newObserver;
    }

private:

    ObserverMap _stats;
};

class ObjectObserverUpdater : public IceUtil::Shared
{
public:

    virtual void update() = 0;
};
typedef IceUtil::Handle<ObjectObserverUpdater> ObjectObserverUpdaterPtr;

class ObserverResolverI : public Ice::ObserverResolver
{
public:

    ObserverResolverI(const MetricsAdminIPtr&);

    virtual void setObserverUpdater(const Ice::ObserverUpdaterPtr&);

    virtual Ice::ConnectionObserverPtr 
    getConnectionObserver(const Ice::ConnectionObserverPtr&, const Ice::ConnectionPtr&);

    virtual Ice::ObjectObserverPtr 
    getThreadObserver(const Ice::ObjectObserverPtr&, const std::string&, const std::string&);

    virtual Ice::ThreadPoolThreadObserverPtr 
    getThreadPoolThreadObserver(const Ice::ThreadPoolThreadObserverPtr&, const std::string&, const std::string&);

    virtual Ice::RequestObserverPtr 
    getInvocationObserver(const Ice::RequestObserverPtr&, const Ice::ObjectPrx&, const std::string&);

    virtual Ice::RequestObserverPtr 
    getDispatchObserver(const Ice::RequestObserverPtr&, const Ice::ObjectPtr&, const Ice::Current&);

private:

    const MetricsAdminIPtr _metrics;

    ObjectObserverResolver<ConnectionObserverI> _connections;
};

}

#endif
