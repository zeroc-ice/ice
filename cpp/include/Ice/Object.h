// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OBJECT_H
#define ICE_OBJECT_H

#include <IceUtil/GCShared.h>
#include <IceUtil/Mutex.h>
#include <Ice/ObjectF.h>
#include <Ice/ProxyF.h>
#include <Ice/IncomingAsyncF.h>
#include <Ice/Current.h>
#include <Ice/StreamF.h>

namespace IceInternal
{

class Incoming;
class BasicStream;

enum DispatchStatus
{
    DispatchOK,
    DispatchUserException,
    DispatchObjectNotExist,
    DispatchFacetNotExist,
    DispatchOperationNotExist,
    DispatchUnknownLocalException,
    DispatchUnknownUserException,
    DispatchUnknownException,
    DispatchAsync // "Pseudo dispatch status", used internally only to indicate async dispatch.
};

}

namespace Ice
{

class ICE_API Object : virtual public ::IceUtil::GCShared
{
public:

    Object();

    virtual bool operator==(const Object&) const;
    virtual bool operator!=(const Object&) const;
    virtual bool operator<(const Object&) const;
    virtual ::Ice::Int ice_hash() const;

    virtual bool ice_isA(const ::std::string&, const Current& = Current()) const;
    ::IceInternal::DispatchStatus ___ice_isA(::IceInternal::Incoming&, const Current&);

    virtual void ice_ping(const Current&  = Current()) const;
    ::IceInternal::DispatchStatus ___ice_ping(::IceInternal::Incoming&, const Current&);

    virtual ::std::vector< ::std::string> ice_ids(const Current& = Current()) const;
    ::IceInternal::DispatchStatus ___ice_ids(::IceInternal::Incoming&, const Current&);

    virtual const ::std::string& ice_id(const Current& = Current()) const;
    ::IceInternal::DispatchStatus ___ice_id(::IceInternal::Incoming&, const Current&);

    static const ::std::string& ice_staticId();

    void __copyMembers(::Ice::ObjectPtr) const;
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

    static ::std::string __all[];
    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const Current&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);

    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);

    virtual void __gcReachable(::IceUtil::GCObjectMultiSet&) const {}
    virtual void __gcClear() {}
};

class ICE_API Blobject : virtual public Object
{
public:

    // Returns true if ok, false if user exception.
    virtual bool ice_invoke(const std::vector<Byte>&, std::vector<Byte>&, const Current&) = 0;
    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const Current&);
};

class ICE_API BlobjectAsync : virtual public Object
{
public:

    // Returns true if ok, false if user exception.
    virtual void ice_invoke_async(const AMD_Object_ice_invokePtr&, const std::vector<Byte>&, const Current&) = 0;
    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const Current&);
};

ICE_API void ice_writeObject(const OutputStreamPtr&, const ObjectPtr&);
ICE_API void ice_readObject(const InputStreamPtr&, ObjectPtr&);

}

#endif
