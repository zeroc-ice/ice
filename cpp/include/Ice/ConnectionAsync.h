// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

    virtual void completed(const ::Ice::AsyncResultPtr& iceResult) const
    {
        ::Ice::ConnectionPtr iceCon = iceResult->getConnection();
        assert(iceCon);
        try
        {
            iceCon->end_flushBatchRequests(iceResult);
            assert(false);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::CallbackNC<T>::exception(iceResult, ex);
        }
    }
};

template<class T> Callback_Connection_flushBatchRequestsPtr
newCallback_Connection_flushBatchRequests(const IceUtil::Handle<T>& instance,
                                          void (T::*excb)(const ::Ice::Exception&),
                                          void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Connection_flushBatchRequests<T>(instance, excb, sentcb);
}

template<class T> Callback_Connection_flushBatchRequestsPtr
newCallback_Connection_flushBatchRequests(T* instance, void (T::*excb)(const ::Ice::Exception&),
                                          void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Connection_flushBatchRequests<T>(instance, excb, sentcb);
}

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

    virtual void completed(const ::Ice::AsyncResultPtr& iceResult) const
    {
        ::Ice::ConnectionPtr iceCon = iceResult->getConnection();
        assert(iceCon);
        try
        {
            iceCon->end_flushBatchRequests(iceResult);
            assert(false);
        }
        catch(const ::Ice::Exception& ex)
        {
            ::IceInternal::Callback<T, CT>::exception(iceResult, ex);
        }
    }
};

template<class T, typename CT> Callback_Connection_flushBatchRequestsPtr
newCallback_Connection_flushBatchRequests(const IceUtil::Handle<T>& instance,
                                          void (T::*excb)(const ::Ice::Exception&, const CT&),
                                          void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Connection_flushBatchRequests<T, CT>(instance, excb, sentcb);
}

template<class T, typename CT> Callback_Connection_flushBatchRequestsPtr
newCallback_Connection_flushBatchRequests(T* instance, void (T::*excb)(const ::Ice::Exception&, const CT&),
                                          void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Connection_flushBatchRequests<T, CT>(instance, excb, sentcb);
}

}

#endif
#endif
