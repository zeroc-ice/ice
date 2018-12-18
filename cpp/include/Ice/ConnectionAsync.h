// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONNECTION_ASYNC_H
#define ICE_CONNECTION_ASYNC_H

#ifndef ICE_CPP11_MAPPING

#include <Ice/Connection.h>
#include <Ice/Proxy.h>

namespace Ice
{

/**
 * Type-safe asynchronous callback wrapper class used for calls to
 * Ice::Connection::begin_flushBatchRequests.
 * Create a wrapper instance by calling ::Ice::newCallback_Connection_flushBatchRequests.
 * \headerfile Ice/Ice.h
 */
template<class T>
class CallbackNC_Connection_flushBatchRequests : public Callback_Connection_flushBatchRequests_Base,
                                                 public ::IceInternal::OnewayCallbackNC<T>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&);
    typedef void (T::*Sent)(bool);

    CallbackNC_Connection_flushBatchRequests(const TPtr& obj, Exception excb, Sent sentcb)
        : ::IceInternal::OnewayCallbackNC<T>(obj, 0, excb, sentcb)
    {
    }

    /// \cond INTERNAL
    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        ::Ice::ConnectionPtr connection = result->getConnection();
        assert(connection);
        try
        {
            connection->end_flushBatchRequests(result);
            assert(false);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::CallbackNC<T>::exception(result, ex);
        }
    }
    /// \endcond
};

/**
 * Creates a callback wrapper instance that delegates to your object.
 * @param instance The callback object.
 * @param excb The exception method of the callback object.
 * @param sentcb The sent method of the callback object.
 * @return An object that can be passed to an asynchronous invocation of
 * Ice::Connection::begin_flushBatchRequests.
 */
template<class T> Callback_Connection_flushBatchRequestsPtr
newCallback_Connection_flushBatchRequests(const IceUtil::Handle<T>& instance,
                                          void (T::*excb)(const ::Ice::Exception&),
                                          void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Connection_flushBatchRequests<T>(instance, excb, sentcb);
}

/**
 * Creates a callback wrapper instance that delegates to your object.
 * @param instance The callback object.
 * @param excb The exception method of the callback object.
 * @param sentcb The sent method of the callback object.
 * @return An object that can be passed to an asynchronous invocation of
 * Ice::Connection::begin_flushBatchRequests.
 */
template<class T> Callback_Connection_flushBatchRequestsPtr
newCallback_Connection_flushBatchRequests(T* instance, void (T::*excb)(const ::Ice::Exception&),
                                          void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Connection_flushBatchRequests<T>(instance, excb, sentcb);
}

/**
 * Type-safe asynchronous callback wrapper class used for calls to
 * Ice::Connection::begin_flushBatchRequests.
 * Create a wrapper instance by calling ::Ice::newCallback_Connection_flushBatchRequests.
 * \headerfile Ice/Ice.h
 */
template<class T, typename CT>
class Callback_Connection_flushBatchRequests : public Callback_Connection_flushBatchRequests_Base,
                                               public ::IceInternal::OnewayCallback<T, CT>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception& , const CT&);
    typedef void (T::*Sent)(bool , const CT&);

    Callback_Connection_flushBatchRequests(const TPtr& obj, Exception excb, Sent sentcb)
        : ::IceInternal::OnewayCallback<T, CT>(obj, 0, excb, sentcb)
    {
    }

    /// \cond INTERNAL
    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        ::Ice::ConnectionPtr connection = result->getConnection();
        assert(connection);
        try
        {
            connection->end_flushBatchRequests(result);
            assert(false);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::Callback<T, CT>::exception(result, ex);
        }
    }
    /// \endcond
};

/**
 * Creates a callback wrapper instance that delegates to your object.
 * @param instance The callback object.
 * @param excb The exception method of the callback object.
 * @param sentcb The sent method of the callback object.
 * @return An object that can be passed to an asynchronous invocation of
 * Ice::Connection::begin_flushBatchRequests.
 */
template<class T, typename CT> Callback_Connection_flushBatchRequestsPtr
newCallback_Connection_flushBatchRequests(const IceUtil::Handle<T>& instance,
                                          void (T::*excb)(const ::Ice::Exception&, const CT&),
                                          void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Connection_flushBatchRequests<T, CT>(instance, excb, sentcb);
}

/**
 * Creates a callback wrapper instance that delegates to your object.
 * @param instance The callback object.
 * @param excb The exception method of the callback object.
 * @param sentcb The sent method of the callback object.
 * @return An object that can be passed to an asynchronous invocation of
 * Ice::Connection::begin_flushBatchRequests.
 */
template<class T, typename CT> Callback_Connection_flushBatchRequestsPtr
newCallback_Connection_flushBatchRequests(T* instance, void (T::*excb)(const ::Ice::Exception&, const CT&),
                                          void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Connection_flushBatchRequests<T, CT>(instance, excb, sentcb);
}

/**
 * Type-safe asynchronous callback wrapper class used for calls to
 * Ice::Connection::begin_heartbeat.
 * Create a wrapper instance by calling ::Ice::newCallback_Connection_heartbeat.
 * \headerfile Ice/Ice.h
 */
template<class T>
class CallbackNC_Connection_heartbeat : public Callback_Connection_heartbeat_Base,
                                        public ::IceInternal::OnewayCallbackNC<T>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&);
    typedef void (T::*Sent)(bool);

    CallbackNC_Connection_heartbeat(const TPtr& obj, Exception excb, Sent sentcb)
        : ::IceInternal::OnewayCallbackNC<T>(obj, 0, excb, sentcb)
    {
    }

    /// \cond INTERNAL
    virtual void completed(const ::Ice::AsyncResultPtr& __result) const
    {
        ::Ice::ConnectionPtr __con = __result->getConnection();
        assert(__con);
        try
        {
            __con->end_heartbeat(__result);
            assert(false);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::CallbackNC<T>::exception(__result, ex);
        }
    }
    /// \endcond
};

/**
 * Creates a callback wrapper instance that delegates to your object.
 * @param instance The callback object.
 * @param excb The exception method of the callback object.
 * @param sentcb The sent method of the callback object.
 * @return An object that can be passed to an asynchronous invocation of
 * Ice::Connection::begin_heartbeat.
 */
template<class T> Callback_Connection_heartbeatPtr
newCallback_Connection_heartbeat(const IceUtil::Handle<T>& instance,
                                 void (T::*excb)(const ::Ice::Exception&),
                                 void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Connection_heartbeat<T>(instance, excb, sentcb);
}

/**
 * Creates a callback wrapper instance that delegates to your object.
 * @param instance The callback object.
 * @param excb The exception method of the callback object.
 * @param sentcb The sent method of the callback object.
 * @return An object that can be passed to an asynchronous invocation of
 * Ice::Connection::begin_heartbeat.
 */
template<class T> Callback_Connection_heartbeatPtr
newCallback_Connection_heartbeat(T* instance, void (T::*excb)(const ::Ice::Exception&), void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Connection_heartbeat<T>(instance, excb, sentcb);
}

/**
 * Type-safe asynchronous callback wrapper class used for calls to
 * Ice::Connection::begin_heartbeat.
 * Create a wrapper instance by calling ::Ice::newCallback_Connection_heartbeat.
 * \headerfile Ice/Ice.h
 */
template<class T, typename CT>
class Callback_Connection_heartbeat : public Callback_Connection_heartbeat_Base,
                                      public ::IceInternal::OnewayCallback<T, CT>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception& , const CT&);
    typedef void (T::*Sent)(bool , const CT&);

    Callback_Connection_heartbeat(const TPtr& obj, Exception excb, Sent sentcb)
        : ::IceInternal::OnewayCallback<T, CT>(obj, 0, excb, sentcb)
    {
    }

    /// \cond INTERNAL
    virtual void completed(const ::Ice::AsyncResultPtr& __result) const
    {
        ::Ice::ConnectionPtr __con = __result->getConnection();
        assert(__con);
        try
        {
            __con->end_heartbeat(__result);
            assert(false);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::Callback<T, CT>::exception(__result, ex);
        }
    }
    /// \endcond
};

/**
 * Creates a callback wrapper instance that delegates to your object.
 * @param instance The callback object.
 * @param excb The exception method of the callback object.
 * @param sentcb The sent method of the callback object.
 * @return An object that can be passed to an asynchronous invocation of
 * Ice::Connection::begin_heartbeat.
 */
template<class T, typename CT> Callback_Connection_heartbeatPtr
newCallback_Connection_heartbeat(const IceUtil::Handle<T>& instance,
                                 void (T::*excb)(const ::Ice::Exception&, const CT&),
                                 void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Connection_heartbeat<T, CT>(instance, excb, sentcb);
}

/**
 * Creates a callback wrapper instance that delegates to your object.
 * @param instance The callback object.
 * @param excb The exception method of the callback object.
 * @param sentcb The sent method of the callback object.
 * @return An object that can be passed to an asynchronous invocation of
 * Ice::Connection::begin_heartbeat.
 */
template<class T, typename CT> Callback_Connection_heartbeatPtr
newCallback_Connection_heartbeat(T* instance, void (T::*excb)(const ::Ice::Exception&, const CT&),
                                 void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Connection_heartbeat<T, CT>(instance, excb, sentcb);
}

}

#endif
#endif
