// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_H
#define ICE_OBJECT_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/ObjectF.h>
#include <Ice/ProxyF.h>
#include <Ice/StreamF.h>
#include <Ice/Current.h>

namespace IceInternal
{

class Incoming;
class BasicStream;

enum DispatchStatus
{
    DispatchOK,
    DispatchUserException,
    DispatchLocationForward,
    DispatchObjectNotExist,
    DispatchFacetNotExist,
    DispatchOperationNotExist,
    DispatchUnknownLocalException,
    DispatchUnknownUserException,
    DispatchUnknownException
};

}

namespace Ice
{

class ICE_API LocationForward
{
public:

    LocationForward(const LocationForward&);
    LocationForward(const ObjectPrx&);

protected:

    ObjectPrx _prx;
    friend class ::IceProxy::Ice::Object;
    friend class ::IceInternal::Incoming;
};

class ICE_API Object : virtual public ::IceUtil::Shared
{
public:

    Object();

    virtual bool operator==(const Object&) const;
    virtual bool operator!=(const Object&) const;
    virtual bool operator<(const Object&) const;
    virtual ::Ice::Int ice_hash() const;

    static const ::std::string __ids[];

    virtual bool ice_isA(const ::std::string&, const Current& = Current());
    ::IceInternal::DispatchStatus ___ice_isA(::IceInternal::Incoming&, const Current&);

    virtual void ice_ping(const Current&  = Current());
    ::IceInternal::DispatchStatus ___ice_ping(::IceInternal::Incoming&, const Current&);

    virtual ::std::vector< ::std::string> ice_ids(const Current& = Current());
    ::IceInternal::DispatchStatus ___ice_ids(::IceInternal::Incoming&, const Current&);

    virtual const ::std::string& ice_id(const Current& = Current());
    ::IceInternal::DispatchStatus ___ice_id(::IceInternal::Incoming&, const Current&);

    ::std::vector< ::std::string> ice_facets(const Current& = Current());
    ::IceInternal::DispatchStatus ___ice_facets(::IceInternal::Incoming&, const Current&);

    static const ::std::string& ice_staticId();

    static ::std::string __all[];
    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const Current&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*);

    virtual void __marshal(const ::Ice::StreamPtr&) const;
    virtual void __unmarshal(const ::Ice::StreamPtr&);

    void ice_marshal(const ::std::string&, const ::Ice::StreamPtr&);
    static void ice_unmarshal(const ::std::string&, const ::Ice::StreamPtr&, ObjectPtr&);

    void ice_addFacet(const ObjectPtr&, const ::std::string&);
    void ice_removeFacet(const ::std::string&);
    void ice_removeAllFacets();
    ObjectPtr ice_findFacet(const ::std::string&);

private:

    std::map<std::string, ObjectPtr> _activeFacetMap;
    std::map<std::string, ObjectPtr>::iterator _activeFacetMapHint;
    ::IceUtil::Mutex _activeFacetMapMutex;
};

class ICE_API Blobject : virtual public Object
{
public:

    // Returns true if ok, false if user exception.
    virtual bool ice_invoke(const std::vector<Byte>&, std::vector<Byte>&, const Current&) = 0;
    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const Current&);
};

}

#endif
