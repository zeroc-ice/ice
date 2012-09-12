// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/InstrumentationI.h>

#include <Ice/LocalException.h>
#include <Ice/Communicator.h>
#include <Ice/LoggerUtil.h>

using namespace std;
using namespace Ice;
using namespace IceStorm::Instrumentation;
using namespace IceMX;

namespace 
{

class TopicHelper : public MetricsHelperT<TopicMetrics>
{
public:

    class Attributes : public AttributeResolverT<TopicHelper>
    {
    public:
        
        Attributes()
        {
            add("parent", &TopicHelper::getService);
            add("id", &TopicHelper::getId);
            add("topic", &TopicHelper::getId);
            add("service", &TopicHelper::getService);
        }
    };
    static Attributes attributes;
    
    TopicHelper(const string& service, const string& name) : _service(service), _name(name)
    {
    }

    virtual string operator()(const string& attribute) const
    {
        return attributes(this, attribute);
    }

    const string& getService() const
    {
        return _service;
    }

    const string& getId() const
    {
        return _name;
    }
    
private:

    const string& _service;
    const string& _name;
};

TopicHelper::Attributes TopicHelper::attributes;

class SubscriberHelper : public MetricsHelperT<SubscriberMetrics>
{
public:

    class Attributes : public AttributeResolverT<SubscriberHelper>
    {
    public:
        
        Attributes()
        {
            add("parent", &SubscriberHelper::getTopic);
            add("id", &SubscriberHelper::getId);
            add("topic", &SubscriberHelper::getTopic);
            add("service", &SubscriberHelper::getService);

            add("identity", &SubscriberHelper::getIdentity);
            add("facet", &SubscriberHelper::getProxy, &IceProxy::Ice::Object::ice_getFacet);
            add("encoding", &SubscriberHelper::getProxy, &IceProxy::Ice::Object::ice_getEncodingVersion);
            add("mode", &SubscriberHelper::getMode);
            add("proxy", &SubscriberHelper::getProxy);
            add("isLink", &SubscriberHelper::_isLink);

            setDefault(&SubscriberHelper::resolve);
        }
    };
    static Attributes attributes;
    
    SubscriberHelper(const string& svc, const string& topic, const Ice::ObjectPrx& proxy, const IceStorm::QoS& qos, 
                     bool isLink) :
        _service(svc), _topic(topic), _proxy(proxy), _qos(qos), _isLink(isLink)
    {
    }

    virtual string operator()(const string& attribute) const
    {
        return attributes(this, attribute);
    }

    string resolve(const string& attribute) const
    {
        if(attribute.compare(0, 4, "qos.") == 0)
        {
            IceStorm::QoS::const_iterator p = _qos.find(attribute.substr(4));
            if(p != _qos.end())
            {
                return p->second;
            }
            else
            {
                return "default";
            }
        }
        return "unknown";
    }

    const string&
    getService() const
    {
        return _service;
    }

    const string&
    getTopic() const
    {
        return _topic;
    }

    string
    getMode() const
    {
        if(_proxy->ice_isTwoway())
        {
            return "twoway";
        } 
        else if(_proxy->ice_isOneway())
        {
            return "oneway";
        } 
        else if(_proxy->ice_isBatchOneway())
        {
            return "batch-oneway";
        } 
        else if(_proxy->ice_isDatagram())
        {
            return "datagram";
        } 
        else if(_proxy->ice_isBatchDatagram())
        {
            return "batch-datagram";
        } 
        else
        {
            return "unknown";
        }
    }

    const string&
    getId() const
    {
        if(_id.empty())
        {
            try
            {
                _id = _proxy->ice_toString();
            }
            catch(const Ice::FixedProxyException& ex)
            {
                _id = _proxy->ice_getCommunicator()->identityToString(_proxy->ice_getIdentity());
            }
        }
        return _id;
    }

    const ObjectPrx&
    getProxy() const
    {
        return _proxy;
    }

    string
    getIdentity() const
    {
        return _proxy->ice_getCommunicator()->identityToString(_proxy->ice_getIdentity());
    }

private:

    const string& _service;
    const string& _topic;
    const Ice::ObjectPrx& _proxy;
    const IceStorm::QoS& _qos;
    bool _isLink;
    mutable string _id;
};

SubscriberHelper::Attributes SubscriberHelper::attributes;

}

void
TopicObserverI::published()
{
    forEach(inc(&TopicMetrics::published));
}

void
TopicObserverI::forwarded()
{
    forEach(inc(&TopicMetrics::forwarded));
}

void
SubscriberObserverI::queued(int count)
{
    struct Update
    {
        Update(int count) : count(count)
        {
        }

        void operator()(const SubscriberMetricsPtr& v)
        {
            v->queued += count;
        }

        int count;
    };
    forEach(Update(count));
}

void
SubscriberObserverI::outstanding(int count)
{
    struct Update
    {
        Update(int count) : count(count)
        {
        }

        void operator()(const SubscriberMetricsPtr& v)
        {
            v->queued -= count;
            v->outstanding += count;
        }

        int count;
    };
    forEach(Update(count));
}

void
SubscriberObserverI::delivered(int count)
{
    struct Update
    {
        Update(int count) : count(count)
        {
        }

        void operator()(const SubscriberMetricsPtr& v)
        {
            v->outstanding -= count;
            v->delivered += count;
        }

        int count;
    };
    forEach(Update(count));
}

TopicManagerObserverI::TopicManagerObserverI(const MetricsAdminIPtr& metrics) : 
    _metrics(metrics),
    _topics(metrics, "Topic"),
    _subscribers(metrics, "Subscriber")
{
    _metrics->updateViews();
}

void
TopicManagerObserverI::setObserverUpdater(const ObserverUpdaterPtr& updater)
{
    _topics.setUpdater(newUpdater(updater, &ObserverUpdater::updateTopicObservers));
    _subscribers.setUpdater(newUpdater(updater, &ObserverUpdater::updateSubscriberObservers));
}

TopicObserverPtr
TopicManagerObserverI::getTopicObserver(const string& service, const string& topic, const TopicObserverPtr& old)
{
    if(_topics.isEnabled())
    {
        try
        {
            return _topics.getObserver(TopicHelper(service, topic), old);
        }
        catch(const exception& ex)
        {
            Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return 0;
}

SubscriberObserverPtr
TopicManagerObserverI::getSubscriberObserver(const string& svc, 
                                             const string& topic, 
                                             const Ice::ObjectPrx& proxy, 
                                             const IceStorm::QoS& qos,
                                             bool isLink,
                                             const SubscriberObserverPtr& old)
{
    if(_subscribers.isEnabled())
    {
        try
        {
            return _subscribers.getObserver(SubscriberHelper(svc, topic, proxy, qos, isLink), old);
        }
        catch(const exception& ex)
        {
            Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return 0;
}

