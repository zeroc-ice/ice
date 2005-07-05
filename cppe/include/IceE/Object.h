// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_H
#define ICEE_OBJECT_H

#include <IceE/ObjectF.h>
#include <IceE/Current.h>
#include <IceE/DispatchStatus.h>

namespace IceEInternal
{

class Incoming;
class BasicStream;

}

namespace IceE
{

class ICEE_API Object : public IceE::Shared
{
public:

    Object() {};

    virtual bool operator==(const Object&) const;
    virtual bool operator!=(const Object&) const;
    virtual bool operator<(const Object&) const;

    virtual Int ice_hash() const;

    virtual bool ice_isA(const std::string&, const Current& = Current()) const;
    IceEInternal::DispatchStatus ___ice_isA(IceEInternal::Incoming&, const Current&);

    virtual void ice_ping(const Current&  = Current()) const;
    IceEInternal::DispatchStatus ___ice_ping(IceEInternal::Incoming&, const Current&);

    virtual std::vector< std::string> ice_ids(const Current& = Current()) const;
    IceEInternal::DispatchStatus ___ice_ids(IceEInternal::Incoming&, const Current&);

    virtual const std::string& ice_id(const Current& = Current()) const;
    IceEInternal::DispatchStatus ___ice_id(IceEInternal::Incoming&, const Current&);

    static const std::string& ice_staticId();

    virtual ObjectPtr ice_clone() const;

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

    static std::string __all[];
    virtual IceEInternal::DispatchStatus __dispatch(IceEInternal::Incoming&, const Current&);

protected:

    virtual ~Object() {} // This class is abstract.
};

class ICEE_API Blobject : public Object
{
public:

    // Returns true if ok, false if user exception.
    virtual bool ice_invoke(const std::vector<Byte>&, std::vector<Byte>&, const Current&) = 0;
    virtual IceEInternal::DispatchStatus __dispatch(IceEInternal::Incoming&, const Current&);
};

}

#endif
