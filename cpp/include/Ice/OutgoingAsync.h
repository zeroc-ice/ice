// Copyright (c) ZeroC, Inc.

#ifndef ICE_OUTGOING_ASYNC_H
#define ICE_OUTGOING_ASYNC_H

#include "CommunicatorF.h"
#include "ConnectionF.h"
#include "ConnectionIF.h"
#include "InputStream.h"
#include "LocalExceptions.h"
#include "ObjectAdapterF.h"
#include "ObserverHelper.h"
#include "OutputStream.h"
#include "Proxy.h"
#include "RequestHandlerF.h"
#include "TimerTask.h"

#include <cassert>
#include <exception>
#include <string_view>

#if defined(__clang__)
#    pragma clang diagnostic push
// See #2747
#    pragma clang diagnostic ignored "-Wshadow-uncaptured-local"
#    pragma clang diagnostic ignored "-Wweak-vtables"
#endif

namespace IceInternal
{
    class OutgoingAsyncBase;
    class RetryException;
    class CollocatedRequestHandler;

    enum AsyncStatus
    {
        AsyncStatusQueued = 0,
        AsyncStatusSent = 1,
        AsyncStatusInvokeSentCallback = 2
    };

    class ICE_API OutgoingAsyncCompletionCallback
    {
    public:
        virtual ~OutgoingAsyncCompletionCallback();

    protected:
        // Returns true if handleInvokeSent handles sent callbacks.
        virtual bool handleSent(bool done, bool alreadySent) noexcept = 0;

        // Returns true if handleInvokeException handles exception callbacks.
        virtual bool handleException(std::exception_ptr) noexcept = 0;

        // Returns true if handleInvokeResponse handles response callbacks.
        // This function can unmarshal the response and throw an exception.
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

        void
        attachRemoteObserver(const Ice::ConnectionInfoPtr& c, const Ice::EndpointPtr& endpt, std::int32_t requestId);

        void attachCollocatedObserver(const Ice::ObjectAdapterPtr& adapter, std::int32_t requestId);

        Ice::OutputStream* getOs() { return &_os; }

        Ice::InputStream* getIs() { return &_is; }

    protected:
        OutgoingAsyncBase(const InstancePtr&);

        bool sentImpl(bool);
        bool exceptionImpl(std::exception_ptr);
        bool responseImpl(bool, bool);

        void cancel(std::exception_ptr);

        void warning(std::string_view callbackName, std::exception_ptr eptr) const;

        //
        // This virtual method is necessary for the communicator flush
        // batch requests implementation.
        //
        virtual IceInternal::InvocationObserver& getObserver() { return _observer; }

        const InstancePtr _instance;
        Ice::ConnectionPtr _cachedConnection;
        bool _sentSynchronously{false};
        bool _doneInSent{false};
        unsigned char _state{0};

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

    using OutgoingAsyncBasePtr = std::shared_ptr<OutgoingAsyncBase>;

    //
    // Base class for proxy based invocations. This class handles the
    // retry for proxy invocations. It also ensures the child observer is
    // correct notified of failures and make sure the retry task is
    // correctly canceled when the invocation completes.
    //
    class ICE_API ProxyOutgoingAsyncBase : public OutgoingAsyncBase, public TimerTask
    {
    public:
        virtual AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool) = 0;
        virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*) = 0;

        bool exception(std::exception_ptr) override;

        void retryException();
        void retry();
        void abort(std::exception_ptr);

        std::shared_ptr<ProxyOutgoingAsyncBase> shared_from_this()
        {
            return std::static_pointer_cast<ProxyOutgoingAsyncBase>(OutgoingAsyncBase::shared_from_this());
        }

    protected:
        ProxyOutgoingAsyncBase(Ice::ObjectPrx);
        ~ProxyOutgoingAsyncBase() override;

        void invokeImpl(bool);
        bool sentImpl(bool);
        bool exceptionImpl(std::exception_ptr);
        bool responseImpl(bool, bool);

        void runTimerTask() override;

        const Ice::ObjectPrx _proxy;
        RequestHandlerPtr _handler;
        Ice::OperationMode _mode{Ice::OperationMode::Normal};

    private:
        int handleRetryAfterException(std::exception_ptr);
        int checkRetryAfterException(std::exception_ptr);

        int _cnt{0};
        bool _sent{false};
    };

    using ProxyOutgoingAsyncBasePtr = std::shared_ptr<ProxyOutgoingAsyncBase>;

    //
    // Class for handling Slice operation invocations
    //
    class ICE_API OutgoingAsync : public ProxyOutgoingAsyncBase
    {
    public:
        OutgoingAsync(Ice::ObjectPrx, bool);

        void prepare(std::string_view operation, Ice::OperationMode mode, const Ice::Context& context);

        bool sent() override;
        bool response() override;

        AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool) override;
        AsyncStatus invokeCollocated(CollocatedRequestHandler*) override;

        void abort(std::exception_ptr);
        void invoke(std::string_view);
        void invoke(
            std::string_view,
            Ice::OperationMode,
            std::optional<Ice::FormatType>,
            const Ice::Context&,
            const std::function<void(Ice::OutputStream*)>&);
        void throwUserException();

        Ice::OutputStream* startWriteParams(std::optional<Ice::FormatType> format)
        {
            _os.startEncapsulation(_encoding, format);
            return &_os;
        }
        void endWriteParams() { _os.endEncapsulation(); }
        void writeEmptyParams() { _os.writeEmptyEncapsulation(_encoding); }
        void writeParamEncaps(const std::byte* encaps, std::int32_t size)
        {
            if (size == 0)
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
        std::function<void(const Ice::UserException&)> _userException;
        bool _synchronous;
    };

    using OutgoingAsyncPtr = std::shared_ptr<OutgoingAsync>;

    class ICE_API LambdaInvoke : public virtual OutgoingAsyncCompletionCallback
    {
    public:
        LambdaInvoke(std::function<void(std::exception_ptr)> exception, std::function<void(bool)> sent)
            : _exception(std::move(exception)),
              _sent(std::move(sent))
        {
        }

    protected:
        bool handleSent(bool, bool) noexcept final;
        bool handleException(std::exception_ptr) noexcept final;
        bool handleResponse(bool) final;

        void handleInvokeSent(bool, OutgoingAsyncBase*) const final;
        void handleInvokeException(std::exception_ptr, OutgoingAsyncBase*) const final;
        void handleInvokeResponse(bool, OutgoingAsyncBase*) const final;

        std::function<void(std::exception_ptr)> _exception;
        std::function<void(bool)> _sent;
        std::function<void(bool)> _response;
    };

    template<typename R> class PromiseInvoke : public virtual OutgoingAsyncCompletionCallback
    {
    public:
        [[nodiscard]] std::future<R> getFuture() { return _promise.get_future(); }

    protected:
        bool handleSent(bool, bool) noexcept override { return false; }

        bool handleException(std::exception_ptr ex) noexcept final
        {
            _promise.set_exception(ex);
            return false;
        }

        bool handleResponse(bool ok) final
        {
            _response(ok);
            return false;
        }

        void handleInvokeSent(bool, OutgoingAsyncBase*) const final { assert(false); }

        void handleInvokeException(std::exception_ptr, OutgoingAsyncBase*) const final { assert(false); }

        void handleInvokeResponse(bool, OutgoingAsyncBase*) const final { assert(false); }

        std::promise<R> _promise;
        std::function<void(bool)> _response;
    };

    template<typename T> class OutgoingAsyncT : public OutgoingAsync
    {
    public:
        using OutgoingAsync::OutgoingAsync;

        void invoke(
            std::string_view operation,
            Ice::OperationMode mode,
            std::optional<Ice::FormatType> format,
            const Ice::Context& ctx,
            const std::function<void(Ice::OutputStream*)>& write,
            std::function<void(const Ice::UserException&)> userException)
        {
            _read = [](Ice::InputStream* stream)
            {
                T v;
                stream->read(v);
                return v;
            };
            _userException = std::move(userException);
            OutgoingAsync::invoke(operation, mode, format, ctx, write);
        }

        void invoke(
            std::string_view operation,
            Ice::OperationMode mode,
            std::optional<Ice::FormatType> format,
            const Ice::Context& ctx,
            const std::function<void(Ice::OutputStream*)>& write,
            std::function<void(const Ice::UserException&)> userException,
            std::function<T(Ice::InputStream*)> read)
        {
            _read = std::move(read);
            _userException = std::move(userException);
            OutgoingAsync::invoke(operation, mode, format, ctx, write);
        }

    protected:
        std::function<T(Ice::InputStream*)> _read;
    };

    template<> class OutgoingAsyncT<void> : public OutgoingAsync
    {
    public:
        using OutgoingAsync::OutgoingAsync;

        void invoke(
            std::string_view operation,
            Ice::OperationMode mode,
            std::optional<Ice::FormatType> format,
            const Ice::Context& ctx,
            const std::function<void(Ice::OutputStream*)>& write,
            std::function<void(const Ice::UserException&)> userException)
        {
            _userException = std::move(userException);
            OutgoingAsync::invoke(operation, mode, format, ctx, write);
        }
    };

    template<typename R> class LambdaOutgoing : public OutgoingAsyncT<R>, public LambdaInvoke
    {
    public:
        LambdaOutgoing(
            Ice::ObjectPrx proxy,
            std::function<void(R)> response,
            std::function<void(std::exception_ptr)> ex,
            std::function<void(bool)> sent)
            : OutgoingAsyncT<R>(std::move(proxy), false),
              LambdaInvoke(std::move(ex), std::move(sent))
        {
            _response = [this, response = std::move(response)](bool ok)
            {
                if (!ok)
                {
                    this->throwUserException();
                }
                else if (response)
                {
                    assert(this->_read);
                    this->_is.startEncapsulation();
                    R v = this->_read(&this->_is);
                    this->_is.endEncapsulation();
                    try
                    {
                        response(std::move(v));
                    }
                    catch (...)
                    {
                        this->warning("response", std::current_exception());
                    }
                }
            };
        }
    };

    template<> class LambdaOutgoing<void> : public OutgoingAsyncT<void>, public LambdaInvoke
    {
    public:
        LambdaOutgoing(
            Ice::ObjectPrx proxy,
            std::function<void()> response,
            std::function<void(std::exception_ptr)> ex,
            std::function<void(bool)> sent)
            : OutgoingAsyncT<void>(std::move(proxy), false),
              LambdaInvoke(std::move(ex), std::move(sent))
        {
            _response = [this, response = std::move(response)](bool ok)
            {
                if (!ok)
                {
                    this->throwUserException();
                }
                else if (response)
                {
                    if (!this->_is.b.empty())
                    {
                        this->_is.skipEmptyEncapsulation();
                    }
                    try
                    {
                        response();
                    }
                    catch (...)
                    {
                        this->warning("response", std::current_exception());
                    }
                }
            };
        }
    };

    template<typename R> class PromiseOutgoing : public OutgoingAsyncT<R>, public PromiseInvoke<R>
    {
    public:
        PromiseOutgoing(Ice::ObjectPrx proxy, bool sync) : OutgoingAsyncT<R>(std::move(proxy), sync)
        {
            this->_response = [this](bool ok)
            {
                if (ok)
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

    template<> class PromiseOutgoing<void> : public OutgoingAsyncT<void>, public PromiseInvoke<void>
    {
    public:
        PromiseOutgoing(Ice::ObjectPrx proxy, bool sync) : OutgoingAsyncT<void>(std::move(proxy), sync)
        {
            this->_response = [&](bool ok)
            {
                if (this->_is.b.empty())
                {
                    //
                    // If there's no response (oneway, batch-oneway proxies), we just set the promise
                    // on completion without reading anything from the input stream. This is required for
                    // batch invocations.
                    //
                    this->_promise.set_value();
                }
                else if (ok)
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

        bool handleSent(bool done, bool) noexcept final
        {
            if (done)
            {
                PromiseInvoke<void>::_promise.set_value();
            }
            return false;
        }
    };

    template<typename R, typename Obj, typename Fn, typename... Args>
    [[nodiscard]] inline std::future<R> makePromiseOutgoing(bool sync, Obj obj, Fn fn, Args&&... args)
    {
        auto outAsync = std::make_shared<PromiseOutgoing<R>>(*obj, sync);
        (obj->*fn)(outAsync, std::forward<Args>(args)...);
        return outAsync->getFuture();
    }

    template<typename R, typename Re, typename E, typename S, typename Obj, typename Fn, typename... Args>
    [[nodiscard]] inline std::function<void()> makeLambdaOutgoing(Re r, E e, S s, Obj obj, Fn fn, Args&&... args)
    {
        auto outAsync = std::make_shared<LambdaOutgoing<R>>(*obj, std::move(r), std::move(e), std::move(s));
        (obj->*fn)(outAsync, std::forward<Args>(args)...);
        return [outAsync]() { outAsync->cancel(); };
    }
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

#endif
