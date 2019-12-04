//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef REQUEST_H
#define REQUEST_H

#include <Ice/Ice.h>

#include <Glacier2/Instrumentation.h>

#include <deque>

namespace Glacier2
{

class Instance;
class Request;
class RequestQueueThread;

class Request
{
public:

    Request(std::shared_ptr<Ice::ObjectPrx>,
            const std::pair<const Ice::Byte*, const Ice::Byte*>&,
            const Ice::Current&,
            bool,
            const Ice::Context&,
            std::function<void(bool, std::pair<const Ice::Byte*, const Ice::Byte*>)>,
            std::function<void(std::exception_ptr)>);

    void invoke(std::function<void(bool, std::pair<const Ice::Byte*, const Ice::Byte*>)>&&,
                std::function<void(std::exception_ptr)>&&,
                std::function<void(bool)>&& = nullptr);
    bool override(const std::shared_ptr<Request>&) const;
    bool hasOverride() const { return !_override.empty(); }

private:

    friend class RequestQueue;
    void response(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&);
    void exception(std::exception_ptr);
    void queued();

    const std::shared_ptr<Ice::ObjectPrx> _proxy;
    const Ice::ByteSeq _inParams;
    const Ice::Current _current;
    const bool _forwardContext;
    const Ice::Context _sslContext;
    const std::string _override;
    std::function<void(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&)> _response;
    std::function<void(std::exception_ptr)> _exception;
};

class RequestQueue : public std::enable_shared_from_this<RequestQueue>
{
public:

    RequestQueue(std::shared_ptr<RequestQueueThread>, std::shared_ptr<Instance>, std::shared_ptr<Ice::Connection>);

    bool addRequest(std::shared_ptr<Request>);
    void flushRequests();

    void destroy();

    void updateObserver(std::shared_ptr<Glacier2::Instrumentation::SessionObserver>);

private:

    void flush();

    void response(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&, const std::shared_ptr<Request>&);
    void exception(std::exception_ptr, const std::shared_ptr<Request>&);
    void sent(bool, const std::shared_ptr<Request>&);

    const std::shared_ptr<RequestQueueThread> _requestQueueThread;
    const std::shared_ptr<Instance> _instance;
    const std::shared_ptr<Ice::Connection> _connection;

    std::deque<std::shared_ptr<Request>> _requests;
    bool _pendingSend;
    std::shared_ptr<Request> _pendingSendRequest;
    bool _destroyed;
    std::shared_ptr<Glacier2::Instrumentation::SessionObserver> _observer;

    std::mutex _mutex;
};

class RequestQueueThread
{
public:

    RequestQueueThread(std::chrono::milliseconds);
    ~RequestQueueThread();

    void flushRequestQueue(std::shared_ptr<RequestQueue>);
    void destroy();

private:
    void run();

    const std::chrono::milliseconds _sleepTime;
    bool _destroy;
    bool _sleep;

    std::chrono::nanoseconds _sleepDuration;
    std::vector<std::shared_ptr<RequestQueue>> _queues;

    std::mutex _mutex;
    std::condition_variable _condVar;
    std::thread _thread;
};

}

#endif
