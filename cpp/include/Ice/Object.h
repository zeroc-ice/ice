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
#include <Ice/ObjectF.h>
#include <Ice/ProxyF.h>

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

struct Current;

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

class ICE_API ProxyRequested
{
};

class ICE_API Object : virtual public ::IceUtil::Shared
{
public:

    Object();

    bool operator==(const Object&) const;
    bool operator<(const Object&) const;
    ::Ice::Int ice_hash() const;

    static const char* __classIds[];
    virtual const char** __getClassIds();

    virtual bool ice_isA(const ::std::string&, const Current&);
    ::IceInternal::DispatchStatus ___ice_isA(::IceInternal::Incoming&, const Current&);

    virtual void ice_ping(const Current&);
    ::IceInternal::DispatchStatus ___ice_ping(::IceInternal::Incoming&, const Current&);

    static const char* __all[];
    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const Current&);
    virtual bool __isMutating(const std::string&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*);

    void ice_addFacet(const ObjectPtr&, const ::std::string&);
    void ice_removeFacet(const ::std::string&);
    void ice_removeAllFacets();
    ObjectPtr ice_findFacet(const ::std::string&);

private:

    std::map<std::string, ObjectPtr> _activeFacetMap;
    std::map<std::string, ObjectPtr>::iterator _activeFacetMapHint;
    JTCMutex _activeFacetMapMutex;
};

class ICE_API Blobject : virtual public Object
{
public:

    virtual void ice_invoke(const std::vector<Byte>&, std::vector<Byte>&, const Current&) = 0;
    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const Current&);
};

}

#endif
