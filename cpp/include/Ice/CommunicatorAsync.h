// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_COMMUNICATOR_ASYNC_H
#define ICE_COMMUNICATOR_ASYNC_H

#include <Ice/Communicator.h>

namespace Ice
{

template<class T>
class CallbackNC_Communicator_flushBatchRequests : public Callback_Communicator_flushBatchRequests_Base,
                                                   public ::IceInternal::OnewayCallbackNC<T>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&);
    typedef void (T::*Sent)(bool);

    CallbackNC_Communicator_flushBatchRequests(const TPtr& obj, Exception excb, Sent sentcb)
        : ::IceInternal::OnewayCallbackNC<T>(obj, 0, excb, sentcb)
    {
    }

    virtual void __completed(const ::Ice::AsyncResultPtr& __result) const
    {
        ::Ice::CommunicatorPtr __com = __result->getCommunicator();
        assert(__com);
        try
        {
            __com->end_flushBatchRequests(__result);
            assert(false);
        }
        catch(::Ice::Exception& ex)
        {
            ::IceInternal::CallbackNC<T>::__exception(__result, ex);
        }
    }
};

#ifdef ICE_CPP11
class Cpp11FnCallbackNC_Communicator_flushBatchRequests : virtual public ::IceInternal::Cpp11FnCallbackNC
{
public:
    
    Cpp11FnCallbackNC_Communicator_flushBatchRequests(
                                            const ::IceInternal::Function<void (const ::Ice::Exception&)>& excb,
                                            const ::IceInternal::Function<void (bool)>& sentcb) :
        ::IceInternal::Cpp11FnCallbackNC(excb, sentcb)
    {
        CallbackBase::checkCallback(true, excb != nullptr);
    }
    
    virtual void
    __completed(const ::Ice::AsyncResultPtr& __result) const
    {
        ::Ice::CommunicatorPtr __com = __result->getCommunicator();
        assert(__com);
        try
        {
            __com->end_flushBatchRequests(__result);
            assert(false);
        }
        catch(::Ice::Exception& ex)
        {
            ::IceInternal::Cpp11FnCallbackNC::__exception(__result, ex);
        }
    }
};
#endif

template<class T> Callback_Communicator_flushBatchRequestsPtr
newCallback_Communicator_flushBatchRequests(const IceUtil::Handle<T>& instance,
                                            void (T::*excb)(const ::Ice::Exception&),
                                            void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Communicator_flushBatchRequests<T>(instance, excb, sentcb);
}

template<class T> Callback_Communicator_flushBatchRequestsPtr
newCallback_Communicator_flushBatchRequests(T* instance, void (T::*excb)(const ::Ice::Exception&),
                                            void (T::*sentcb)(bool) = 0)
{
    return new CallbackNC_Communicator_flushBatchRequests<T>(instance, excb, sentcb);
}

template<class T, typename CT>
class Callback_Communicator_flushBatchRequests : public Callback_Communicator_flushBatchRequests_Base,
                                                 public ::IceInternal::OnewayCallback<T, CT>
{
public:

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception& , const CT&);
    typedef void (T::*Sent)(bool , const CT&);

    Callback_Communicator_flushBatchRequests(const TPtr& obj, Exception excb, Sent sentcb)
        : ::IceInternal::OnewayCallback<T, CT>(obj, 0, excb, sentcb)
    {
    }

    virtual void __completed(const ::Ice::AsyncResultPtr& __result) const
    {
        ::Ice::CommunicatorPtr __com = __result->getCommunicator();
        assert(__com);
        try
        {
            __com->end_flushBatchRequests(__result);
            assert(false);
        }
        catch(::Ice::Exception& ex)
        {
            ::IceInternal::Callback<T, CT>::__exception(__result, ex);
        }
    }
};

template<class T, typename CT> Callback_Communicator_flushBatchRequestsPtr
newCallback_Communicator_flushBatchRequests(const IceUtil::Handle<T>& instance,
                                            void (T::*excb)(const ::Ice::Exception&, const CT&),
                                            void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Communicator_flushBatchRequests<T, CT>(instance, excb, sentcb);
}

template<class T, typename CT> Callback_Communicator_flushBatchRequestsPtr
newCallback_Communicator_flushBatchRequests(T* instance, void (T::*excb)(const ::Ice::Exception&, const CT&),
                                            void (T::*sentcb)(bool, const CT&) = 0)
{
    return new Callback_Communicator_flushBatchRequests<T, CT>(instance, excb, sentcb);
}

}

#endif
