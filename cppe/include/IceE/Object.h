// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_H
#define ICEE_OBJECT_H

#include <IceE/ObjectF.h>
#include <IceE/Current.h>
#include <IceE/DispatchStatus.h>

namespace IceInternal
{

class Incoming;
class BasicStream;

}

namespace Ice
{

class ICE_API Object : public IceUtil::Shared
{
public:

    virtual bool operator==(const Object&) const;
    virtual bool operator!=(const Object&) const;
    virtual bool operator<(const Object&) const;

    virtual Int ice_hash() const;

    virtual bool ice_isA(const std::string&, const Current& = Current()) const;
    IceInternal::DispatchStatus ___ice_isA(IceInternal::Incoming&, const Current&);

    virtual void ice_ping(const Current&  = Current()) const;
    IceInternal::DispatchStatus ___ice_ping(IceInternal::Incoming&, const Current&);

    virtual std::vector< std::string> ice_ids(const Current& = Current()) const;
    IceInternal::DispatchStatus ___ice_ids(IceInternal::Incoming&, const Current&);

    virtual const std::string& ice_id(const Current& = Current()) const;
    IceInternal::DispatchStatus ___ice_id(IceInternal::Incoming&, const Current&);

    static const std::string& ice_staticId();

    virtual ObjectPtr ice_clone() const;

    static std::string __all[];
    virtual IceInternal::DispatchStatus __dispatch(IceInternal::Incoming&, const Current&);

protected:

    Object() {};
    virtual ~Object() {} // This class is abstract.
};

class ICE_API Blobject : public Object
{
public:

    // Returns true if ok, false if user exception.
    virtual bool ice_invoke(const std::vector<Byte>&, std::vector<Byte>&, const Current&) = 0;
    virtual IceInternal::DispatchStatus __dispatch(IceInternal::Incoming&, const Current&);
};

}

#endif
