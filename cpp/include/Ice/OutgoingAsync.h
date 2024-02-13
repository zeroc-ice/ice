//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OUTGOING_ASYNC_H
#define ICE_OUTGOING_ASYNC_H

#include <IceUtil/Timer.h>
#include <IceUtil/Monitor.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/RequestHandlerF.h>
#include <Ice/ConnectionF.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/ObserverHelper.h>
#include <Ice/LocalException.h>

#include <exception>

namespace IceInternal
{

class RetryException;
class CollocatedRequestHandler;

class ICE_API OutgoingAsyncCompletionCallback
{
public:
    virtual ~OutgoingAsyncCompletionCallback();

protected:

    virtual bool handleSent(bool, bool) = 0;
    virtual bool handleException(std::exception_ptr) = 0;
    virtual bool handleResponse(bool) = 0;

    virtual void handleInvokeSent(bool, OutgoingAsyncBase*) const = 0;
    virtual void handleInvokeException(std::exception_ptr, OutgoingAsyncBase*) const = 0;
    virtual void handleInvokeResponse(bool, OutgoingAsyncBase*) const = 0;
};

//
// Base class for handling asynchronous invocations. This class is
// responsible for the handling of the output stream and the child
// invocation observer.
//
class ICE_API OutgoingAsyncBase : public virtual OutgoingAsyncCompletionCallback,
                                  public std::enable_shared_from_this<OutgoingAsyncBase>
{
public:

    virtual bool sent();
    virtual bool exception(std::exception_ptr);
    virtual bool response();

    void invokeSentAsync();
    void invokeExceptionAsync();
    void invokeResponseAsync();

    void invokeSent();
    void invokeException();
    void invokeResponse();

    virtual void cancelable(const IceInternal::CancellationHandlerPtr&);
    void cancel();

    void attachRemoteObserver(const Ice::ConnectionInfoPtr& c, const Ice::EndpointPtr& endpt, Ice::Int requestId)
    {
        const Ice::Int size = static_cast<Ice::Int>(_os.b.size() - headerSize - 4);
        _childObserver.attach(getObserver().getRemoteObserver(c, endpt, requestId, size));
    }

    void attachCollocatedObserver(const Ice::ObjectAdapterPtr& adapter, Ice::Int requestId)
    {
        const Ice::Int size = static_cast<Ice::Int>(_os.b.size() - headerSize - 4);
        _childObserver.attach(getObserver().getCollocatedObserver(adapter, requestId, size));
    }

    Ice::OutputStream* getOs()
    {
        return &_os;
    }

    Ice::InputStream* getIs()
    {
        return &_is;
    }

protected:

    OutgoingAsyncBase(const InstancePtr&);

    bool sentImpl(bool);
    bool exceptionImpl(std::exception_ptr);
    bool responseImpl(bool, bool);

    void cancel(std::exception_ptr);
    void checkCanceled();

    void warning(const std::exception&) const;
    void warning() const;

    //
    // This virtual method is necessary for the communicator flush
    // batch requests implementation.
    //
    virtual IceInternal::InvocationObserver& getObserver()
    {
        return _observer;
    }

    const InstancePtr _instance;
    Ice::ConnectionPtr _cachedConnection;
    bool _sentSynchronously;
    bool _doneInSent;
    unsigned char _state;

    std::mutex _m;
    using Lock = std::lock_guard<std::mutex>;

    std::exception_ptr _ex;
    std::exception_ptr _cancellationException;

    InvocationObserver _observer;
    ObserverHelperT<Ice::Instrumentation::ChildInvocationObserver> _childObserver;

    Ice::OutputStream _os;
    Ice::InputStream _is;

    CancellationHandlerPtr _cancellationHandler;

    static const unsigned char OK;
    static const unsigned char Sent;
};

//
// Base class for proxy based invocations. This class handles the
// retry for proxy invocations. It also ensures the child observer is
// correct notified of failures and make sure the retry task is
// correctly canceled when the invocation completes.
//
class ICE_API ProxyOutgoingAsyncBase : public OutgoingAsyncBase,
                                       public IceUtil::TimerTask
{
public:

    virtual AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool) = 0;
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*) = 0;

    virtual bool exception(std::exception_ptr);
    virtual void cancelable(const CancellationHandlerPtr&);

    void retryException();
    void retry();
    void abort(std::exception_ptr);

    std::shared_ptr<ProxyOutgoingAsyncBase> shared_from_this()
    {
        return std::static_pointer_cast<ProxyOutgoingAsyncBase>(OutgoingAsyncBase::shared_from_this());
    }

protected:

    ProxyOutgoingAsyncBase(const Ice::ObjectPrxPtr&);
    ~ProxyOutgoingAsyncBase();

    void invokeImpl(bool);
    bool sentImpl(bool);
    bool exceptionImpl(std::exception_ptr);
    bool responseImpl(bool, bool);

    virtual void runTimerTask();

    const Ice::ObjectPrxPtr _proxy;
    RequestHandlerPtr _handler;
    Ice::OperationMode _mode;

private:

    int _cnt;
    bool _sent;
};

//
// Class for handling Slice operation invocations
//
class ICE_API OutgoingAsync : public ProxyOutgoingAsyncBase
{
public:

    OutgoingAsync(const Ice::ObjectPrxPtr&, bool);

    void prepare(const std::string&, Ice::OperationMode, const Ice::Context&);

    virtual bool sent();
    virtual bool response();

    virtual AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool);
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*);

    void abort(std::exception_ptr);
    void invoke(const std::string&);
    void invoke(const std::string&, Ice::OperationMode, Ice::FormatType, const Ice::Context&,
                std::function<void(Ice::OutputStream*)>);
    void throwUserException();

    Ice::OutputStream* startWriteParams(Ice::FormatType format)
    {
        _os.startEncapsulation(_encoding, format);
        return &_os;
    }
    void endWriteParams()
    {
        _os.endEncapsulation();
    }
    void writeEmptyParams()
    {
        _os.writeEmptyEncapsulation(_encoding);
    }
    void writeParamEncaps(const ::Ice::Byte* encaps, ::Ice::Int size)
    {
        if(size == 0)
        {
            _os.writeEmptyEncapsulation(_encoding);
        }
        else
        {
            _os.writeEncapsulation(encaps, size);
        }
    }

protected:

    const Ice::EncodingVersion _encoding;
    std::function<void(const ::Ice::UserException&)> _userException;
    bool _synchronous;
};

}

namespace IceInternal
{

class ICE_API LambdaInvoke : public virtual OutgoingAsyncCompletionCallback
{
public:

    LambdaInvoke(std::function<void(::std::exception_ptr)> exception, std::function<void(bool)> sent) :
        _exception(std::move(exception)), _sent(std::move(sent))
    {
    }

protected:

    virtual bool handleSent(bool, bool) override;
    virtual bool handleException(std::exception_ptr) override;
    virtual bool handleResponse(bool) override;

    virtual void handleInvokeSent(bool, OutgoingAsyncBase*) const override;
    virtual void handleInvokeException(std::exception_ptr, OutgoingAsyncBase*) const override;
    virtual void handleInvokeResponse(bool, OutgoingAsyncBase*) const override;

    std::function<void(::std::exception_ptr)> _exception;
    std::function<void(bool)> _sent;
    std::function<void(bool)> _response;
};

template<typename Promise>
class PromiseInvoke : public virtual OutgoingAsyncCompletionCallback
{
public:

    auto
    getFuture() -> decltype(std::declval<Promise>().get_future())
    {
        return _promise.get_future();
    }

protected:

    Promise _promise;
    std::function<void(bool)> _response;

private:

    virtual bool handleSent(bool, bool) override
    {
        return false;
    }

    virtual bool handleException(std::exception_ptr ex) override
    {
        _promise.set_exception(ex);
        return false;
    }

    virtual bool handleResponse(bool ok) override
    {
        _response(ok);
        return false;
    }

    virtual void handleInvokeSent(bool, OutgoingAsyncBase*) const override
    {
        assert(false);
    }

    virtual void handleInvokeException(std::exception_ptr, OutgoingAsyncBase*) const override
    {
        assert(false);
    }

    virtual void handleInvokeResponse(bool, OutgoingAsyncBase*) const override
    {
        assert(false);
    }
};

template<typename T>
class OutgoingAsyncT : public OutgoingAsync
{
public:

    using OutgoingAsync::OutgoingAsync;

    void
    invoke(const std::string& operation,
           Ice::OperationMode mode,
           Ice::FormatType format,
           const Ice::Context& ctx,
           std::function<void(Ice::OutputStream*)> write,
           std::function<void(const Ice::UserException&)> userException)
    {
        _read = [](Ice::InputStream* stream)
        {
            T v;
            stream->read(v);
            return v;
        };
        _userException = std::move(userException);
        OutgoingAsync::invoke(operation, mode, format, ctx, std::move(write));
    }

    void
    invoke(const std::string& operation,
           Ice::OperationMode mode,
           Ice::FormatType format,
           const Ice::Context& ctx,
           std::function<void(Ice::OutputStream*)> write,
           std::function<void(const Ice::UserException&)> userException,
           std::function<T(Ice::InputStream*)> read)
    {
        _read = std::move(read);
        _userException = std::move(userException);
        OutgoingAsync::invoke(operation, mode, format, ctx, std::move(write));
    }

protected:

    std::function<T(Ice::InputStream*)> _read;
};

template<>
class OutgoingAsyncT<void> : public OutgoingAsync
{
public:

    using OutgoingAsync::OutgoingAsync;

    void
    invoke(const std::string& operation,
           Ice::OperationMode mode,
           Ice::FormatType format,
           const Ice::Context& ctx,
           std::function<void(Ice::OutputStream*)> write,
           std::function<void(const Ice::UserException&)> userException)
    {
        _userException = std::move(userException);
        OutgoingAsync::invoke(operation, mode, format, ctx, std::move(write));
    }
};

template<typename R>
class LambdaOutgoing : public OutgoingAsyncT<R>, public LambdaInvoke
{
public:

    LambdaOutgoing(const Ice::ObjectPrxPtr& proxy,
                   std::function<void(R)> response,
                   std::function<void(::std::exception_ptr)> ex,
                   std::function<void(bool)> sent) :
        OutgoingAsyncT<R>(proxy, false), LambdaInvoke(std::move(ex), std::move(sent))
    {
        _response = [this, response = std::move(response)](bool ok)
        {
            if(!ok)
            {
                this->throwUserException();
            }
            else if(response)
            {
                assert(this->_read);
                this->_is.startEncapsulation();
                R v = this->_read(&this->_is);
                this->_is.endEncapsulation();
                try
                {
                    response(std::move(v));
                }
                catch(...)
                {
                    throw std::current_exception();
                }
            }
        };
    }
};

template<>
class LambdaOutgoing<void> : public OutgoingAsyncT<void>, public LambdaInvoke
{
public:

    LambdaOutgoing(const Ice::ObjectPrxPtr& proxy,
                   std::function<void()> response,
                   std::function<void(::std::exception_ptr)> ex,
                   std::function<void(bool)> sent) :
        OutgoingAsyncT<void>(proxy, false), LambdaInvoke(std::move(ex), std::move(sent))
    {
        _response = [this, response = std::move(response)](bool ok)
        {
            if(!ok)
            {
                this->throwUserException();
            }
            else if(response)
            {
                if(!this->_is.b.empty())
                {
                    this->_is.skipEmptyEncapsulation();
                }

                try
                {
                    response();
                }
                catch(...)
                {
                    throw std::current_exception();
                }
            }
        };
    }
};

class CustomLambdaOutgoing : public OutgoingAsync, public LambdaInvoke
{
public:

    CustomLambdaOutgoing(const Ice::ObjectPrxPtr& proxy,
                         std::function<void(Ice::InputStream*)> read,
                         std::function<void(::std::exception_ptr)> ex,
                         std::function<void(bool)> sent) :
        OutgoingAsync(proxy, false), LambdaInvoke(std::move(ex), std::move(sent))
    {
        _response = [this, read = std::move(read)](bool ok)
        {
            if(!ok)
            {
                this->throwUserException();
            }
            else if(read)
            {
                //
                // Read and respond
                //
                read(&this->_is);
            }
        };
    }

    void
    invoke(const std::string& operation,
           Ice::OperationMode mode,
           Ice::FormatType format,
           const Ice::Context& ctx,
           std::function<void(Ice::OutputStream*)> write,
           std::function<void(const Ice::UserException&)> userException)
    {
        _userException = std::move(userException);
        OutgoingAsync::invoke(operation, mode, format, ctx, std::move(write));
    }
};

template<typename P, typename R>
class PromiseOutgoing : public OutgoingAsyncT<R>, public PromiseInvoke<P>
{
public:

    PromiseOutgoing(const Ice::ObjectPrxPtr& proxy, bool sync) :
        OutgoingAsyncT<R>(proxy, sync)
    {
        this->_response = [this](bool ok)
        {
            if(ok)
            {
                assert(this->_read);
                this->_is.startEncapsulation();
                R v = this->_read(&this->_is);
                this->_is.endEncapsulation();
                this->_promise.set_value(std::move(v));
            }
            else
            {
                this->throwUserException();
            }
        };
    }
};

template<typename P>
class PromiseOutgoing<P, void> : public OutgoingAsyncT<void>, public PromiseInvoke<P>
{
public:

    PromiseOutgoing(const Ice::ObjectPrxPtr& proxy, bool sync) :
        OutgoingAsyncT<void>(proxy, sync)
    {
        this->_response = [&](bool ok)
        {
            if(this->_is.b.empty())
            {
                //
                // If there's no response (oneway, batch-oneway proxies), we just set the promise
                // on completion without reading anything from the input stream. This is required for
                // batch invocations.
                //
                this->_promise.set_value();
            }
            else if(ok)
            {
                this->_is.skipEmptyEncapsulation();
                this->_promise.set_value();
            }
            else
            {
                this->throwUserException();
            }
        };
    }

    virtual bool handleSent(bool done, bool) override
    {
        if(done)
        {
            PromiseInvoke<P>::_promise.set_value();
        }
        return false;
    }
};

}

#endif
