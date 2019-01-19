//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ASYNC_RESULT_H
#define ICE_ASYNC_RESULT_H

#ifndef ICE_CPP11_MAPPING

#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>
#include <Ice/LocalObject.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ConnectionF.h>
#include <Ice/ProxyF.h>
#include <Ice/AsyncResultF.h>

namespace Ice
{

/**
 * Represents the result of an asynchronous invocation using the C++98 mapping.
 * \headerfile Ice/Ice.h
 */
class ICE_API AsyncResult : private IceUtil::noncopyable, public Ice::LocalObject
{
public:

    virtual ~AsyncResult();

    /**
     * Prevents a queued invocation from being sent or, if the invocation has already been sent,
     * ignores a reply if the server sends one. cancel is a local operation and has no effect
     * on the server. A canceled invocation is considered to be completed, meaning isCompleted
     * returns true, and the result of the invocation is an InvocationCanceledException.
     */
    virtual void cancel() = 0;

    /**
     * Allows you to create ordered or hashed collections of pending asynchronous invocations.
     * @return A unique hash code for this object.
     */
    virtual Int getHash() const = 0;

    /**
     * Obtains the communicator that sent the invocation.
     * @return A reference to the communicator.
     */
    virtual CommunicatorPtr getCommunicator() const = 0;

    /**
     * Obtains the connection that was used for the invocation. Note that, for typical asynchronous
     * proxy invocations, this method returns a nil value because the possibility of automatic retries
     * means the connection that is currently in use could change unexpectedly. The getConnection
     * method only returns a non-nil value when the AsyncResult object is obtained by calling
     * Connection::begin_flushBatchRequests.
     * @return A reference to the connection.
     */
    virtual ConnectionPtr getConnection() const = 0;

    /**
     * Obtains the proxy that was used to call the begin_ method, or nil if the AsyncResult object was
     * not obtained via an asynchronous proxy invocation.
     * @return A reference to the proxy.
     */
    virtual ObjectPrxPtr getProxy() const = 0;

    /**
     * Obtains the completion status of the invocation.
     * @return True if, at the time it is called, the result of an invocation is available, indicating
     * that a call to the end_ method will not block the caller. Otherwise, if the result is not yet
     * available, the method returns false.
     */
    virtual bool isCompleted() const = 0;

    /**
     * Blocks the caller until the result of an invocation becomes available.
     */
    virtual void waitForCompleted() = 0;

    /**
     * Obtains the sent status of the invocation.
     * @return True if, at the time it is called, the request has been written to the local transport
     * (whether it was initially queued or not). Otherwise, if the request is still queued or an
     * exception occurred before the request could be sent, this method returns false.
     */
    virtual bool isSent() const = 0;

    /**
     * Blocks the calling thread until a request has been written to the client-side transport,
     * or an exception occurs.
     */
    virtual void waitForSent() = 0;

    /**
     * Throws the local exception that caused the invocation to fail. If no exception has occurred yet,
     * this method does nothing.
     */
    virtual void throwLocalException() const = 0;

    /**
     * Determines whether the request was sent synchronously.
     * @return True if a request was written to the client-side transport without first being queued.
     * If the request was initially queued, sentSynchronously returns false (independent of whether
     * the request is still in the queue or has since been written to the client-side transport).
     */
    virtual bool sentSynchronously() const = 0;

    /**
     * Obtains the cookie that was passed to the begin_ method.
     * @return The cookie, or nil if you did not pass a cookie to the begin_ method.
     */
    virtual LocalObjectPtr getCookie() const = 0;

    /**
     * Obtains the name of the operation.
     * @return The operation name.
     */
    virtual const std::string& getOperation() const = 0;

    /// \cond INTERNAL
    virtual bool _waitForResponse() = 0;
    virtual Ice::InputStream* _startReadParams() = 0;
    virtual void _endReadParams() = 0;
    virtual void _readEmptyParams() = 0;
    virtual void _readParamEncaps(const ::Ice::Byte*&, ::Ice::Int&) = 0;
    virtual void _throwUserException() = 0;

    static void _check(const AsyncResultPtr&, const ::IceProxy::Ice::Object*, const ::std::string&);
    static void _check(const AsyncResultPtr&, const Connection*, const ::std::string&);
    static void _check(const AsyncResultPtr&, const Communicator*, const ::std::string&);

    class Callback : public IceUtil::Shared
    {
    public:

        virtual void run() = 0;
    };
    typedef IceUtil::Handle<Callback> CallbackPtr;

    virtual void _scheduleCallback(const CallbackPtr&) = 0;
    /// \endcond

protected:

    /// \cond INTERNAL
    static void check(const AsyncResultPtr&, const ::std::string&);
    /// \endcond
};

}

#endif

#endif
