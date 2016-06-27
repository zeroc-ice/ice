// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

class ICE_API AMDCallback : public virtual Ice::LocalObject
{
public:

    virtual ~AMDCallback();


    virtual void ice_exception(const ::std::exception&) = 0;
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
#ifdef ICE_CPP11_MAPPING
public:

    //
    // The constructor is public but it shouldn't be used directly, use create() instead.
    //
    IncomingAsync(Incoming&);

    static IncomingAsyncPtr create(Incoming&); // Adopts the argument. It must not be used afterwards.

#else

protected:

    IncomingAsync(Incoming&);
#endif

public:

    void __deactivate(Incoming&);

    virtual void ice_exception(const ::std::exception&);
    virtual void ice_exception();

    void __response();
    void __exception(const std::exception&);
    void __exception();

    bool __validateResponse(bool);

private:

    //
    // We need a separate InstancePtr, because _is and _os only hold a
    // Instance* for optimization.
    //
    const InstancePtr _instanceCopy;

    //
    // We need a separate ConnectionIPtr, because IncomingBase only
    // holds a ConnectionI* for optimization.
    //
    const ResponseHandlerPtr _responseHandlerCopy;

    const bool _retriable;
    bool _active;
};

}

#ifndef ICE_CPP11_MAPPING
namespace Ice
{

class ICE_API AMD_Object_ice_invoke : public virtual Ice::AMDCallback
{
public:

    virtual ~AMD_Object_ice_invoke();

    virtual void ice_response(bool, const std::vector<Ice::Byte>&) = 0;
    virtual void ice_response(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&) = 0;
};

}

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
#endif

#endif
