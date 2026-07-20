// Copyright (c) ZeroC, Inc.

#include "Subscriber.h"
#include "Ice/LoggerUtil.h"
#include "Ice/StringUtil.h"
#include "Instance.h"
#include "NodeI.h"
#include "TraceLevels.h"
#include "Util.h"

#include <iterator>
#include <stdexcept>

using namespace std;
using namespace IceStorm;
using namespace IceStormElection;

//
// Per Subscriber object.
//
namespace
{
    class PerSubscriberPublisherI final : public Ice::BlobjectArray
    {
    public:
        PerSubscriberPublisherI(shared_ptr<Instance> instance) : _instance(std::move(instance)) {}

        void setSubscriber(shared_ptr<Subscriber> subscriber) { _subscriber = std::move(subscriber); }

        bool ice_invoke(pair<const byte*, const byte*> inParams, vector<byte>&, const Ice::Current& current) override
        {
            // Use cached reads.
            CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);

            EventData event = {current.operation, current.mode, Ice::ByteSeq(), current.ctx};

            Ice::ByteSeq data(inParams.first, inParams.second);
            event.data.swap(data);

            EventDataSeq e;
            e.push_back(std::move(event));
            _subscriber->queue(false, std::move(e));
            return true;
        }

    private:
        const shared_ptr<Instance> _instance;
        shared_ptr<Subscriber> _subscriber;
    };

    IceStorm::Instrumentation::SubscriberState toSubscriberState(Subscriber::SubscriberState s)
    {
        switch (s)
        {
            case Subscriber::SubscriberStateOnline:
                return IceStorm::Instrumentation::SubscriberState::SubscriberStateOnline;
            case Subscriber::SubscriberStateOffline:
                return IceStorm::Instrumentation::SubscriberState::SubscriberStateOffline;
            case Subscriber::SubscriberStateError:
            case Subscriber::SubscriberStateReaped:
                return IceStorm::Instrumentation::SubscriberState::SubscriberStateError;
            default:
                assert(false);
                return IceStorm::Instrumentation::SubscriberState::SubscriberStateError;
        }
    }

    // Parses str (ignoring surrounding whitespace) as a base-10 int consumed in full; nullopt otherwise.
    optional<int> toInt(const string& str)
    {
        const string s = IceInternal::trim(str);
        try
        {
            size_t pos = 0;
            int value = stoi(s, &pos);
            if (pos == s.size())
            {
                return value;
            }
        }
        catch (const std::exception&)
        {
        }
        return nullopt;
    }
}

// Each of the various Subscriber types.
namespace
{
    class SubscriberBatch final : public Subscriber
    {
    public:
        SubscriberBatch(
            const shared_ptr<Instance>&,
            const SubscriberRecord&,
            const Ice::ObjectPrx&,
            int,
            Ice::ObjectPrx);

        void flush() override;

        // Called on the flush timer to deliver the queued events as a single batch.
        void doFlush();

    private:
        // Called once the batch has been written to the transport.
        void batchSent();

        const Ice::ObjectPrx _obj;
        const std::chrono::milliseconds _interval;
    };

    class SubscriberOneway final : public Subscriber
    {
    public:
        SubscriberOneway(
            const shared_ptr<Instance>&,
            const SubscriberRecord&,
            const Ice::ObjectPrx&,
            int,
            Ice::ObjectPrx);

        void flush() override;
        void sentAsynchronously();

    private:
        const Ice::ObjectPrx _obj;
    };

    class SubscriberTwoway final : public Subscriber
    {
    public:
        SubscriberTwoway(
            const shared_ptr<Instance>&,
            const SubscriberRecord&,
            const Ice::ObjectPrx&,
            int,
            int,
            Ice::ObjectPrx);

        void flush() override;

    private:
        const Ice::ObjectPrx _obj;
    };

    class SubscriberLink final : public Subscriber
    {
    public:
        SubscriberLink(const shared_ptr<Instance>&, const SubscriberRecord&);

        void flush() override;

    private:
        const TopicLinkPrx _obj;
    };
}

SubscriberBatch::SubscriberBatch(
    const shared_ptr<Instance>& instance,
    const SubscriberRecord& rec,
    const Ice::ObjectPrx& proxy,
    int retryCount,
    Ice::ObjectPrx obj)
    : Subscriber(instance, rec, proxy, retryCount, 1),
      _obj(std::move(obj)),
      _interval(instance->flushInterval())
{
}

void
SubscriberBatch::flush()
{
    lock_guard lock(_mutex);

    //
    // If the subscriber isn't online, or there's nothing to send, we're done.
    //
    if (_state != SubscriberStateOnline || _events.empty())
    {
        return;
    }

    // Defer delivery: accumulate events and send them together when the flush timer fires. _outstanding doubles as a
    // "flush already scheduled or in flight" guard, so at most one flush is pending at a time (_maxOutstanding is 1).
    if (_outstanding == 0)
    {
        ++_outstanding;
        auto self = static_pointer_cast<SubscriberBatch>(shared_from_this());
        _instance->batchFlusher()->schedule([self] { self->doFlush(); }, _interval);
    }
}

void
SubscriberBatch::doFlush()
{
    lock_guard lock(_mutex);

    if (_state != SubscriberStateOnline)
    {
        // The subscriber went offline or errored (e.g. the send queue filled up) after this flush was scheduled but
        // before it ran: release the flush reservation and wake a waiting shutdown().
        --_outstanding;
        assert(_outstanding >= 0 && _outstanding < _maxOutstanding);
        if (_events.empty() && _shutdown)
        {
            _condVar.notify_one();
        }
        return;
    }

    EventDataSeq v;
    v.swap(_events);
    assert(!v.empty());

    if (_observer)
    {
        _outstandingCount = static_cast<int>(v.size());
        _observer->outstanding(_outstandingCount);
    }

    try
    {
        // Queue the events on the batch proxy, then flush them to the transport as a single batch request.
        vector<byte> dummy;
        for (const auto& e : v)
        {
            _obj->ice_invoke(e.op, e.mode, e.data, dummy, e.context);
        }

        auto self = static_pointer_cast<SubscriberBatch>(shared_from_this());
        _obj->ice_flushBatchRequestsAsync(
            [self](exception_ptr ex) { self->error(true, ex); },
            [self](bool) { self->batchSent(); });
    }
    catch (...)
    {
        // Catch everything, not just std::exception: a configured batch-request interceptor can throw a
        // non-std::exception, which BatchRequestQueue rethrows. Letting it escape would leave _outstanding at 1 and
        // hang shutdown().
        error(true, current_exception());
    }
}

void
SubscriberBatch::batchSent()
{
    lock_guard lock(_mutex);

    // The batch has been written to the transport; the flush is no longer outstanding.
    --_outstanding;
    assert(_outstanding >= 0 && _outstanding < _maxOutstanding);
    if (_observer)
    {
        _observer->delivered(_outstandingCount);
    }

    // A successful send means the subscriber is reachable, so reset the consecutive-retry count.
    _currentRetry = 0;

    if (_events.empty() && _outstanding == 0 && _shutdown)
    {
        _condVar.notify_one();
    }
    else if (!_events.empty())
    {
        // More events were queued while the batch was in flight; schedule the next flush.
        flush();
    }
}

SubscriberOneway::SubscriberOneway(
    const shared_ptr<Instance>& instance,
    const SubscriberRecord& rec,
    const Ice::ObjectPrx& proxy,
    int retryCount,
    Ice::ObjectPrx obj)
    : Subscriber(instance, rec, proxy, retryCount, 5),
      _obj(std::move(obj))
{
}

void
SubscriberOneway::flush()
{
    lock_guard lock(_mutex);

    //
    // If the subscriber isn't online we're done.
    //
    if (_state != SubscriberStateOnline || _events.empty())
    {
        return;
    }

    // Send up to _maxOutstanding pending events.
    while (_outstanding < _maxOutstanding && !_events.empty())
    {
        //
        // Dequeue the head event, count one more outstanding AMI
        // request.
        //
        EventData e = std::move(_events.front());
        _events.pop_front();
        if (_observer)
        {
            _observer->outstanding(1);
        }

        try
        {
            auto self = static_pointer_cast<SubscriberOneway>(shared_from_this());
            auto isSent = make_shared<promise<bool>>();
            auto future = isSent->get_future();

            _obj->ice_invokeAsync(
                e.op,
                e.mode,
                e.data,
                nullptr,
                [self](exception_ptr ex) { self->error(true, ex); },
                [self, isSent](bool sentSynchronously)
                {
                    isSent->set_value(sentSynchronously);
                    if (!sentSynchronously)
                    {
                        self->sentAsynchronously();
                    }
                },
                e.context);

            //
            // Check if the request is (or potentially was) sent asynchronously
            //
            // If the request was sent synchronously then the isSent promise will have been set during the call
            // to ice_invokeAsync (sent callback is called immediately after sending from the current thread).
            //
            // Otherwise if the request was sent asynchronously but quick enough so that the isSent promise is already
            // fulfilled, we need to verify the sent callback's sentSynchronously value
            //
            if (future.wait_for(0s) != future_status::ready || future.get() == false)
            {
                ++_outstanding;
            }
            else
            {
                // The event was delivered synchronously, so the subscriber is reachable: reset the
                // consecutive-retry count as the twoway subscriber does on a successful reply.
                _currentRetry = 0;
                if (_observer)
                {
                    _observer->delivered(1);
                }
            }
        }
        catch (const std::exception&)
        {
            error(false, current_exception());
            return;
        }
    }

    if (_events.empty() && _outstanding == 0 && _shutdown)
    {
        _condVar.notify_one();
    }
}

void
SubscriberOneway::sentAsynchronously()
{
    lock_guard lock(_mutex);

    // Decrement the _outstanding count.
    --_outstanding;
    assert(_outstanding >= 0 && _outstanding < _maxOutstanding);
    // The event was delivered, so the subscriber is reachable: reset the consecutive-retry count as the
    // twoway subscriber does on a successful reply.
    _currentRetry = 0;
    if (_observer)
    {
        _observer->delivered(1);
    }

    if (_events.empty() && _outstanding == 0 && _shutdown)
    {
        _condVar.notify_one();
    }
    else if (_outstanding <= 0 && !_events.empty())
    {
        flush();
    }
}

SubscriberTwoway::SubscriberTwoway(
    const shared_ptr<Instance>& instance,
    const SubscriberRecord& rec,
    const Ice::ObjectPrx& proxy,
    int retryCount,
    int maxOutstanding,
    Ice::ObjectPrx obj)
    : Subscriber(instance, rec, proxy, retryCount, maxOutstanding),
      _obj(std::move(obj))
{
}

void
SubscriberTwoway::flush()
{
    lock_guard lock(_mutex);

    //
    // If the subscriber isn't online we're done.
    //
    if (_state != SubscriberStateOnline || _events.empty())
    {
        return;
    }

    // Send up to _maxOutstanding pending events.
    while (_outstanding < _maxOutstanding && !_events.empty())
    {
        //
        // Dequeue the head event, count one more outstanding AMI
        // request.
        //
        EventData e = std::move(_events.front());
        _events.pop_front();

        ++_outstanding;

        if (_observer)
        {
            _observer->outstanding(1);
        }

        try
        {
            auto self = static_pointer_cast<SubscriberTwoway>(shared_from_this());
            _obj->ice_invokeAsync(
                e.op,
                e.mode,
                e.data,
                [self](bool, const vector<byte>&) { self->completed(); },
                [self](exception_ptr ex) { self->error(true, ex); },
                nullptr,
                e.context);
        }
        catch (const std::exception&)
        {
            error(true, current_exception());
            return;
        }
    }
}

namespace
{
    SubscriberLink::SubscriberLink(const shared_ptr<Instance>& instance, const SubscriberRecord& rec)
        : Subscriber(instance, rec, nullopt, -1, 1),
          _obj(Ice::uncheckedCast<TopicLinkPrx>(rec.obj->ice_invocationTimeout(instance->sendTimeout())))
    {
    }

    void SubscriberLink::flush()
    {
        lock_guard lock(_mutex);

        if (_state != SubscriberStateOnline || _outstanding > 0)
        {
            return;
        }

        EventDataSeq v;
        v.swap(_events);

        auto p = v.begin();
        while (p != v.end())
        {
            if (_rec.cost != 0)
            {
                int cost = 0;
                auto q = p->context.find("cost");
                if (q != p->context.end())
                {
                    cost = toInt(q->second).value_or(0);
                }
                if (cost > _rec.cost)
                {
                    p = v.erase(p);
                    continue;
                }
            }
            ++p;
        }

        if (!v.empty())
        {
            try
            {
                ++_outstanding;
                if (_observer)
                {
                    _outstandingCount = static_cast<int>(v.size());
                    _observer->outstanding(_outstandingCount);
                }

                auto self = shared_from_this();
                _obj->forwardAsync(
                    v,
                    [self]() { self->completed(); },
                    [self](exception_ptr ex) { self->error(true, ex); });
            }
            catch (const std::exception&)
            {
                error(true, current_exception());
            }
        }

        // If every queued event was filtered out by cost, we emptied _events without forwarding anything and no
        // completion callback will fire; wake a waiting shutdown() ourselves, as SubscriberOneway::flush() does.
        if (_events.empty() && _outstanding == 0 && _shutdown)
        {
            _condVar.notify_one();
        }
    }
}

const char*
SendQueueSizeMaxReachedException::ice_id() const noexcept
{
    return "::IceStorm::SendQueueSizeMaxReachedException";
}

shared_ptr<Subscriber>
Subscriber::create(const shared_ptr<Instance>& instance, const SubscriberRecord& rec)
{
    if (rec.link)
    {
        return make_shared<SubscriberLink>(instance, rec);
    }
    else
    {
        auto per = make_shared<PerSubscriberPublisherI>(instance);
        Ice::Identity perId;
        perId.category = instance->instanceName();
        perId.name = "topic." + rec.topicName + ".publish." +
                     instance->communicator()->identityToString(rec.obj->ice_getIdentity());

        // Build the proxy for the per-subscriber publisher servant, but don't add the servant to the publish
        // adapter yet: it must not be reachable until its subscriber is successfully set below.
        auto proxy = instance->publishAdapter()->createProxy(perId);

        int retryCount = 0;
        auto p = rec.theQoS.find("retryCount");
        if (p != rec.theQoS.end())
        {
            if (auto value = toInt(p->second))
            {
                retryCount = *value;
            }
            else
            {
                throw BadQoS("invalid retry count (numeric value required): " + p->second);
            }
        }

        string reliability;
        p = rec.theQoS.find("reliability");
        if (p != rec.theQoS.end())
        {
            reliability = p->second;
        }
        if (!reliability.empty() && reliability != "ordered")
        {
            throw BadQoS("invalid reliability: " + reliability);
        }

        // Override the invocation timeout with the send timeout.
        Ice::ObjectPrx newObj = rec.obj->ice_invocationTimeout(instance->sendTimeout());

        p = rec.theQoS.find("locatorCacheTimeout");
        if (p != rec.theQoS.end())
        {
            auto value = toInt(p->second);
            if (!value)
            {
                throw BadQoS("invalid locator cache timeout (numeric value required): " + p->second);
            }
            newObj = newObj->ice_locatorCacheTimeout(*value);
        }

        p = rec.theQoS.find("connectionCached");
        if (p != rec.theQoS.end())
        {
            auto value = toInt(p->second);
            if (!value)
            {
                throw BadQoS("invalid connection cached setting (numeric value required): " + p->second);
            }
            newObj = newObj->ice_connectionCached(*value > 0);
        }

        shared_ptr<Subscriber> subscriber;
        if (reliability == "ordered")
        {
            if (!newObj->ice_isTwoway())
            {
                throw BadQoS("ordered reliability requires a twoway proxy");
            }
            subscriber = make_shared<SubscriberTwoway>(instance, rec, proxy, retryCount, 1, newObj);
        }
        else if (newObj->ice_isOneway() || newObj->ice_isDatagram())
        {
            subscriber = make_shared<SubscriberOneway>(instance, rec, proxy, retryCount, newObj);
        }
        else if (newObj->ice_isBatchOneway() || newObj->ice_isBatchDatagram())
        {
            subscriber = make_shared<SubscriberBatch>(instance, rec, proxy, retryCount, newObj);
        }
        else // if(newObj->ice_isTwoway())
        {
            assert(newObj->ice_isTwoway());
            subscriber = make_shared<SubscriberTwoway>(instance, rec, proxy, retryCount, 5, newObj);
        }
        per->setSubscriber(subscriber);

        // The subscriber is set, so it's now safe to make the servant dispatchable.
        instance->publishAdapter()->add(per, perId);

        return subscriber;
    }
}

std::optional<Ice::ObjectPrx>
Subscriber::proxy() const
{
    return _proxyReplica;
}

Ice::Identity
Subscriber::id() const
{
    return _rec.id;
}

SubscriberRecord
Subscriber::record() const
{
    return _rec;
}

bool
Subscriber::queue(bool forwarded, EventDataSeq events)
{
    lock_guard lock(_mutex);

    // If this is a link subscriber if the set of events were
    // forwarded from another IceStorm instance then do not queue the
    // events.
    if (forwarded && _rec.link)
    {
        return true;
    }

    switch (_state)
    {
        case SubscriberStateOffline:
        {
            if (chrono::steady_clock::now() < _next)
            {
                break;
            }

            //
            // State transition to online.
            //
            setState(SubscriberStateOnline);
        }
            /* FALLTHROUGH */

        case SubscriberStateOnline:
        {
            for (auto& event : events)
            {
                if (static_cast<int>(_events.size()) == _instance->sendQueueSizeMax())
                {
                    if (_instance->sendQueueSizeMaxPolicy() == Instance::RemoveSubscriber)
                    {
                        error(false, make_exception_ptr(SendQueueSizeMaxReachedException{__FILE__, __LINE__}));
                        return false;
                    }
                    else // DropEvents
                    {
                        _events.pop_front();
                    }
                }
                _events.push_back(std::move(event));
            }

            if (_observer)
            {
                _observer->queued(static_cast<int32_t>(events.size()));
            }
            flush();
            break;
        }
        case SubscriberStateError:
            return false;

        case SubscriberStateReaped:
            break;
    }

    return true;
}

bool
Subscriber::reap()
{
    lock_guard lock(_mutex);
    assert(_state >= SubscriberStateError);
    if (_state == SubscriberStateError)
    {
        setState(SubscriberStateReaped);
        return true;
    }
    return false;
}

void
Subscriber::resetIfReaped()
{
    lock_guard lock(_mutex);

    if (_state == SubscriberStateReaped)
    {
        setState(SubscriberStateError);
    }
}

bool
Subscriber::errored() const
{
    lock_guard lock(_mutex);

    return _state >= SubscriberStateError;
}

void
Subscriber::destroy()
{
    //
    // Clear the per-subscriber object if it exists.
    //
    if (_proxy)
    {
        try
        {
            _instance->publishAdapter()->remove(_proxy->ice_getIdentity());
        }
        catch (const Ice::NotRegisteredException&)
        {
            // Ignore
        }
        catch (const Ice::ObjectAdapterDestroyedException&)
        {
            // Ignore -- this can occur on shutdown.
        }
    }

    lock_guard lock(_mutex);
    _observer.detach();
}

void
Subscriber::completed()
{
    lock_guard lock(_mutex);

    // Decrement the _outstanding count.
    --_outstanding;
    assert(_outstanding >= 0 && _outstanding < _maxOutstanding);
    if (_observer)
    {
        _observer->delivered(_outstandingCount);
    }

    //
    // A successful response means we're no longer retrying, we're
    // back active.
    //
    _currentRetry = 0;

    if (_events.empty() && _outstanding == 0 && _shutdown)
    {
        _condVar.notify_one();
    }
    else
    {
        flush();
    }
}

void
Subscriber::error(bool dec, exception_ptr e)
{
    lock_guard lock(_mutex);

    if (dec)
    {
        // Decrement the _outstanding count.
        --_outstanding;
        assert(_outstanding >= 0 && _outstanding < _maxOutstanding);
    }

    //
    // It's possible to be already in the error state if the queue maximum size
    // has been reached or if an ObjectNotExistException occurred before.
    //
    if (_state >= SubscriberStateError)
    {
        if (_shutdown)
        {
            _condVar.notify_one();
        }
        return;
    }

    // A hard error is an ObjectNotExistException, NotRegisteredException, or SendQueueSizeMaxReachedException
    bool hardError;
    string what;
    try
    {
        rethrow_exception(e);
    }
    catch (const Ice::ObjectNotExistException& ex)
    {
        hardError = true;
        what = ex.what();
    }
    catch (const Ice::NotRegisteredException& ex)
    {
        hardError = true;
        what = ex.what();
    }
    catch (const SendQueueSizeMaxReachedException& ex)
    {
        hardError = true;
        what = ex.what();
    }
    catch (const std::exception& ex)
    {
        hardError = false;
        what = ex.what();
    }
    catch (...)
    {
        hardError = false;
        what = "unknown exception";
    }

    //
    // A twoway subscriber can queue multiple send events and
    // therefore its possible to get multiple error'd replies. Ignore
    // replies if we're retrying and its not yet time to process the
    // next request.
    //
    auto now = std::chrono::steady_clock::now();
    if (!hardError && _state == SubscriberStateOffline && now < _next)
    {
        // _outstanding was decremented above, so wake a waiting shutdown() like the other exit paths do.
        if (_shutdown)
        {
            _condVar.notify_one();
        }
        return;
    }

    //
    // If we're in our retry limits and the error isn't a hard failure
    // (that is ObjectNotExistException or NotRegisteredException)
    // then we transition to an offline state.
    //
    if (!hardError && (_retryCount == -1 || _currentRetry < _retryCount))
    {
        assert(_state < SubscriberStateError);

        auto traceLevels = _instance->traceLevels();
        if (_currentRetry == 0)
        {
            Ice::Warning warn(traceLevels->logger);
            warn << traceLevels->subscriberCat << ":" << _instance->communicator()->identityToString(_rec.id);
            if (traceLevels->subscriber > 1)
            {
                warn << " endpoints: " << IceStormInternal::describeEndpoints(_rec.obj);
            }
            warn << " subscriber offline: " << what << " discarding events: " << _instance->discardInterval().count()
                 << "s retryCount: " << _retryCount;
        }
        else
        {
            if (traceLevels->subscriber > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->subscriberCat);
                out << _instance->communicator()->identityToString(_rec.id);
                if (traceLevels->subscriber > 1)
                {
                    out << " endpoints: " << IceStormInternal::describeEndpoints(_rec.obj);
                }
                out << " subscriber offline: " << what << " discarding events: " << _instance->discardInterval().count()
                    << "s retry: " << _currentRetry << "/" << _retryCount;
            }
        }

        // Transition to offline state, increment the retry count and
        // clear all queued events.
        _next = now + _instance->discardInterval();
        ++_currentRetry;
        _events.clear();
        setState(SubscriberStateOffline);
    }
    // Errored out.
    else if (_state < SubscriberStateError)
    {
        _events.clear();
        setState(SubscriberStateError);

        auto traceLevels = _instance->traceLevels();
        if (traceLevels->subscriber > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->subscriberCat);
            out << _instance->communicator()->identityToString(_rec.id);
            if (traceLevels->subscriber > 1)
            {
                out << " endpoints: " << IceStormInternal::describeEndpoints(_rec.obj);
            }
            out << " subscriber errored out: " << what << " retry: " << _currentRetry << "/" << _retryCount;
        }
    }

    if (_shutdown && _events.empty())
    {
        _condVar.notify_one();
    }
}

void
Subscriber::shutdown()
{
    unique_lock lock(_mutex);

    _shutdown = true;
    _condVar.wait(lock, [this] { return _outstanding == 0 && _events.empty(); });

    _observer.detach();
}

void
Subscriber::updateObserver()
{
    lock_guard lock(_mutex);

    if (_instance->observer())
    {
        assert(_rec.obj);
        _observer.attach(_instance->observer()->getSubscriberObserver(
            _rec.topicName,
            *_rec.obj,
            _rec.theQoS,
            _rec.theTopic,
            toSubscriberState(_state),
            _observer.get()));
    }
}

Subscriber::Subscriber(
    shared_ptr<Instance> instance,
    SubscriberRecord rec,
    optional<Ice::ObjectPrx> proxy,
    int retryCount,
    int maxOutstanding)
    : _instance(std::move(instance)),
      _rec(std::move(rec)),
      _retryCount(retryCount),
      _maxOutstanding(maxOutstanding),
      _proxy(std::move(proxy)),
      _proxyReplica(_proxy)
{
    if (_proxy && _instance->publisherReplicaProxy())
    {
        const_cast<optional<Ice::ObjectPrx>&>(_proxyReplica) =
            _instance->publisherReplicaProxy()->ice_identity(_proxy->ice_getIdentity());
    }

    if (_instance->observer())
    {
        assert(_rec.obj);
        _observer.attach(_instance->observer()->getSubscriberObserver(
            _rec.topicName,
            *_rec.obj,
            _rec.theQoS,
            _rec.theTopic,
            toSubscriberState(_state),
            nullptr));
    }
}

namespace
{
    string stateToString(Subscriber::SubscriberState state)
    {
        switch (state)
        {
            case Subscriber::SubscriberStateOnline:
                return "online";
            case Subscriber::SubscriberStateOffline:
                return "offline";
            case Subscriber::SubscriberStateError:
                return "error";
            case Subscriber::SubscriberStateReaped:
                return "reaped";
            default:
                return "???";
        }
    }
}

void
Subscriber::setState(Subscriber::SubscriberState state)
{
    if (state != _state)
    {
        auto traceLevels = _instance->traceLevels();
        if (traceLevels->subscriber > 1)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->subscriberCat);
            out << "endpoints: " << IceStormInternal::describeEndpoints(_rec.obj)
                << " transition from: " << stateToString(_state) << " to: " << stateToString(state);
        }
        _state = state;

        if (_instance->observer())
        {
            assert(_rec.obj);
            _observer.attach(_instance->observer()->getSubscriberObserver(
                _rec.topicName,
                *_rec.obj,
                _rec.theQoS,
                _rec.theTopic,
                toSubscriberState(_state),
                _observer.get()));
        }
    }
}

bool
IceStorm::operator==(const shared_ptr<Subscriber>& subscriber, const Ice::Identity& id)
{
    return subscriber->id() == id;
}
