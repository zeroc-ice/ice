// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_INCOMING_ASYNC_H
#define ICE_INCOMING_ASYNC_H

#include <Ice/IncomingAsyncF.h>
#include <Ice/Incoming.h>

namespace IceInternal
{

//
// We need virtual inheritance from shared, because we use multiple
// inheritance from IceUtil::Shared for generated AMD code.
//
class ICE_API IncomingAsync : public IncomingBase, virtual public IceUtil::Shared
{
public:

    IncomingAsync(Incoming&); // Adopts the argument. It must not be used afterwards.

    void __deactivate(Incoming&);

protected:

    void __response(bool);
    void __exception(const Ice::Exception&);
    void __exception(const std::exception&);
    void __exception();

    bool __validateResponse(bool);
    bool __validateException(const std::exception&);
    bool __validateException();


    // Inlined for speed optimization.
    BasicStream* __os() { return &_os; }

private:

    bool __servantLocatorFinished();

    //
    // We need a separate InstancePtr, because _is and _os only hold a
    // Instance* for optimization.
    //
    const InstancePtr _instanceCopy;

    //
    // We need a separate ConnectionIPtr, because IncomingBase only
    // holds a ConnectionI* for optimization.
    //
    const Ice::ConnectionIPtr _connectionCopy;

    const bool _retriable;
    bool _active;
};

}

namespace Ice
{

class ICE_API AMD_Object_ice_invoke : virtual public IceUtil::Shared
{
public:
    
    virtual void ice_response(bool, const std::vector<Ice::Byte>&) = 0;
    virtual void ice_exception(const IceUtil::Exception&) = 0;
    virtual void ice_exception(const std::exception&) = 0;
    virtual void ice_exception() = 0;
};

class ICE_API AMD_Array_Object_ice_invoke : virtual public IceUtil::Shared
{
public:
    
    virtual void ice_response(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&) = 0;
    virtual void ice_exception(const IceUtil::Exception&) = 0;
    virtual void ice_exception(const std::exception&) = 0;
    virtual void ice_exception() = 0;
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
    virtual void ice_exception(const IceUtil::Exception&);
    virtual void ice_exception(const std::exception&);
    virtual void ice_exception();
};

class ICE_API AMD_Array_Object_ice_invoke : public ::Ice::AMD_Array_Object_ice_invoke, 
                                            public IceInternal::IncomingAsync
{
public:
    
    AMD_Array_Object_ice_invoke(IceInternal::Incoming&);
    
    virtual void ice_response(bool, const std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&);
    virtual void ice_exception(const IceUtil::Exception&);
    virtual void ice_exception(const std::exception&);
    virtual void ice_exception();
};

}

}

#endif
