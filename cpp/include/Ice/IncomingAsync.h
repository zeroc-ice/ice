// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_INCOMING_ASYNC_H
#define ICE_INCOMING_ASYNC_H

#include <Ice/IncomingAsyncF.h>
#include <Ice/Incoming.h>

#ifndef ICE_CPP11_MAPPING
namespace Ice
{

/**
 * Base class for generated AMD callback classes.
 * \headerfile Ice/Ice.h
 */
class ICE_API AMDCallback : public Ice::LocalObject
{
public:

    virtual ~AMDCallback();

    /**
     * Completes the asynchronous request with the given exception.
     * @param ex The exception that completed the request.
     */
    virtual void ice_exception(const ::std::exception& ex) = 0;

    /**
     * Completes the asynchronous request with an UnknownException.
     */
    virtual void ice_exception() = 0;
};

}
#endif

namespace IceInternal
{

//
// We need virtual inheritance from AMDCallback, because we use multiple
// inheritance from Ice::AMDCallback for generated AMD code.
//
class ICE_API IncomingAsync : public IncomingBase,
#ifdef ICE_CPP11_MAPPING
    public ::std::enable_shared_from_this<IncomingAsync>
#else
    public virtual Ice::AMDCallback
#endif
{
public:

    IncomingAsync(Incoming&);

#ifdef ICE_CPP11_MAPPING

    static std::shared_ptr<IncomingAsync> create(Incoming&);

    std::function<void()> response()
    {
        auto self = shared_from_this();
        return [self]()
        {
            self->writeEmptyParams();
            self->completed();
        };
    }

    template<class T>
    std::function<void(const T&)> response()
    {
        auto self = shared_from_this();
        return [self](const T& marshaledResult)
        {
            self->setMarshaledResult(marshaledResult);
            self->completed();
        };
    }

    std::function<void(std::exception_ptr)> exception()
    {
        auto self = shared_from_this();
        return [self](std::exception_ptr ex) { self->completed(ex); };
    }

#else

    virtual void ice_exception(const ::std::exception&);
    virtual void ice_exception();

#endif

    void kill(Incoming&);

    void completed();

#ifdef ICE_CPP11_MAPPING
    void completed(std::exception_ptr);
#endif

private:

    void checkResponseSent();
    bool _responseSent;

    //
    // We need a separate ConnectionIPtr, because IncomingBase only
    // holds a ConnectionI* for optimization.
    //
    const ResponseHandlerPtr _responseHandlerCopy;
};

}

#ifndef ICE_CPP11_MAPPING
namespace Ice
{

/**
 * Base class for the AMD callback for BlobjectAsync::ice_invoke_async.
 * \headerfile Ice/Ice.h
 */
class ICE_API AMD_Object_ice_invoke : public virtual Ice::AMDCallback
{
public:

    virtual ~AMD_Object_ice_invoke();

    /**
     * Completes the request.
     * @param ok True if the request completed successfully, in which case bytes contains an encapsulation
     * of the marshaled results. False if the request completed with a user exception, in which case bytes
     * contains an encapsulation of the marshaled user exception.
     * @param bytes An encapsulation of the results or user exception.
     */
    virtual void ice_response(bool ok, const std::vector<Ice::Byte>& bytes) = 0;

    /**
     * Completes the request.
     * @param ok True if the request completed successfully, in which case bytes contains an encapsulation
     * of the marshaled results. False if the request completed with a user exception, in which case bytes
     * contains an encapsulation of the marshaled user exception.
     * @param bytes An encapsulation of the results or user exception.
     */
    virtual void ice_response(bool ok, const std::pair<const Ice::Byte*, const Ice::Byte*>& bytes) = 0;
};

}

/// \cond INTERNAL
namespace IceAsync
{

namespace Ice
{

class ICE_API AMD_Object_ice_invoke : public ::Ice::AMD_Object_ice_invoke, public IceInternal::IncomingAsync
{
public:

    AMD_Object_ice_invoke(IceInternal::Incoming&);

    virtual void ice_response(bool, const std::vector< ::Ice::Byte>&);
    virtual void ice_response(bool, const std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&);
};

}

}
/// \endcond
#endif

#endif
