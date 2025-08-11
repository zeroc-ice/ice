// Copyright (c) ZeroC, Inc.

#include "InstrumentationI.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"

#include <stdexcept>

using namespace std;
using namespace IceStorm;
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
            Attributes() noexcept
            {
                add("parent", &TopicHelper::getService);
                add("id", &TopicHelper::getId);
                add("topic", &TopicHelper::getId);
                add("service", &TopicHelper::getService);
            }
        };
        static Attributes attributes;

        TopicHelper(const string& name) : _name(name) {}

        string operator()(const string& attribute) const override { return attributes(this, attribute); }

        [[nodiscard]] const string& getService() const { return _service; }

        [[nodiscard]] const string& getId() const { return _name; }

    private:
        const string _service{"IceStorm"};
        const string& _name;
    };

    TopicHelper::Attributes TopicHelper::attributes;

    class SubscriberHelper : public MetricsHelperT<SubscriberMetrics>
    {
    public:
        class Attributes : public AttributeResolverT<SubscriberHelper>
        {
        public:
            Attributes() noexcept
            {
                add("parent", &SubscriberHelper::getTopic);
                add("id", &SubscriberHelper::getId);
                add("topic", &SubscriberHelper::getTopic);
                add("service", &SubscriberHelper::getService);

                add("identity", &SubscriberHelper::getIdentity);

                add("facet", &SubscriberHelper::getProxy, &Ice::ObjectPrx::ice_getFacet);
                add("encoding", &SubscriberHelper::getProxy, &Ice::ObjectPrx::ice_getEncodingVersion);
                add("mode", &SubscriberHelper::getMode);
                add("proxy", &SubscriberHelper::getProxy);
                add("link", &SubscriberHelper::_link);
                add("state", &SubscriberHelper::getState);

                setDefault(&SubscriberHelper::resolve);
            }
        };
        static Attributes attributes;

        SubscriberHelper(
            const string& topic,
            const Ice::ObjectPrx& proxy,
            const IceStorm::QoS& qos,
            optional<IceStorm::TopicPrx> link,
            SubscriberState state)
            : _topic(topic),
              _proxy(proxy),
              _qos(qos),
              _link(std::move(link)),
              _state(state)
        {
        }

        string operator()(const string& attribute) const override { return attributes(this, attribute); }

        [[nodiscard]] string resolve(const string& attribute) const
        {
            if (attribute.compare(0, 4, "qos.") == 0)
            {
                auto p = _qos.find(attribute.substr(4));
                if (p != _qos.end())
                {
                    return p->second;
                }
                else
                {
                    return "default";
                }
            }
            throw invalid_argument(attribute);
        }

        [[nodiscard]] const string& getService() const { return _service; }

        [[nodiscard]] const string& getTopic() const { return _topic; }

        [[nodiscard]] string getMode() const
        {
            if (_proxy->ice_isTwoway())
            {
                return "twoway";
            }
            else if (_proxy->ice_isOneway())
            {
                return "oneway";
            }
            else if (_proxy->ice_isBatchOneway())
            {
                return "batch-oneway";
            }
            else if (_proxy->ice_isDatagram())
            {
                return "datagram";
            }
            else if (_proxy->ice_isBatchDatagram())
            {
                return "batch-datagram";
            }
            else
            {
                return "unknown";
            }
        }

        [[nodiscard]] const string& getId() const
        {
            if (_id.empty())
            {
                try
                {
                    _id = _proxy->ice_toString();
                }
                catch (const Ice::FixedProxyException&)
                {
                    _id = _proxy->ice_getCommunicator()->identityToString(_proxy->ice_getIdentity());
                }
            }
            return _id;
        }

        [[nodiscard]] const Ice::ObjectPrx& getProxy() const { return _proxy; }

        [[nodiscard]] string getState() const
        {
            switch (_state)
            {
                case SubscriberState::SubscriberStateOnline:
                    return "online";
                case SubscriberState::SubscriberStateOffline:
                    return "offline";
                case SubscriberState::SubscriberStateError:
                    return "error";
                default:
                    assert(false);
                    return "";
            }
        }

        [[nodiscard]] string getIdentity() const
        {
            return _proxy->ice_getCommunicator()->identityToString(_proxy->ice_getIdentity());
        }

    private:
        const string _service{"IceStorm"};
        const string& _topic;
        const Ice::ObjectPrx& _proxy;
        const IceStorm::QoS& _qos;
        const optional<IceStorm::TopicPrx> _link;
        const SubscriberState _state;
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

namespace
{
    struct QueuedUpdate
    {
        QueuedUpdate(int countP) : count(countP) {}

        void operator()(const shared_ptr<SubscriberMetrics>& v) { v->queued += count; }

        int count;
    };
}
void
SubscriberObserverI::queued(int count)
{
    forEach(QueuedUpdate(count));
}

namespace
{
    struct OutstandingUpdate
    {
        OutstandingUpdate(int countP) : count(countP) {}

        void operator()(const shared_ptr<SubscriberMetrics>& v)
        {
            if (v->queued > 0)
            {
                v->queued -= count;
            }
            v->outstanding += count;
        }

        int count;
    };
}

void
SubscriberObserverI::outstanding(int count)
{
    forEach(OutstandingUpdate(count));
}

namespace
{
    struct DeliveredUpdate
    {
        DeliveredUpdate(int countP) : count(countP) {}

        void operator()(const shared_ptr<SubscriberMetrics>& v)
        {
            if (v->outstanding > 0)
            {
                v->outstanding -= count;
            }
            v->delivered += count;
        }

        int count;
    };
}

void
SubscriberObserverI::delivered(int count)
{
    forEach(DeliveredUpdate(count));
}

TopicManagerObserverI::TopicManagerObserverI(const shared_ptr<IceInternal::MetricsAdminI>& metrics)
    : _metrics(metrics),
      _topics(metrics, "Topic"),
      _subscribers(metrics, "Subscriber")
{
}

void
TopicManagerObserverI::setObserverUpdater(const shared_ptr<ObserverUpdater>& updater)
{
    _topics.setUpdater(newUpdater(updater, &ObserverUpdater::updateTopicObservers));
    _subscribers.setUpdater(newUpdater(updater, &ObserverUpdater::updateSubscriberObservers));
}

shared_ptr<TopicObserver>
TopicManagerObserverI::getTopicObserver(const string& topic, const shared_ptr<TopicObserver>& old)
{
    if (_topics.isEnabled())
    {
        try
        {
            return _topics.getObserver(TopicHelper(topic), old);
        }
        catch (const exception& ex)
        {
            Ice::Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return nullptr;
}

shared_ptr<SubscriberObserver>
TopicManagerObserverI::getSubscriberObserver(
    const string& topic,
    const Ice::ObjectPrx& proxy,
    const IceStorm::QoS& qos,
    const optional<IceStorm::TopicPrx>& link,
    SubscriberState state,
    const shared_ptr<SubscriberObserver>& old)
{
    if (_subscribers.isEnabled())
    {
        try
        {
            return _subscribers.getObserver(SubscriberHelper(topic, proxy, qos, link, state), old);
        }
        catch (const exception& ex)
        {
            Ice::Error error(_metrics->getLogger());
            error << "unexpected exception trying to obtain observer:\n" << ex;
        }
    }
    return nullptr;
}
