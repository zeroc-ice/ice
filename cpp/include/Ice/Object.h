// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_H
#define ICE_OBJECT_H

#include <Ice/ObjectF.h>
#include <Ice/Shared.h>

namespace __Ice
{

class Incoming;
class Stream;

}

namespace Ice
{

// No virtual inheritance for ice objects
class ICE_API Object : public ::__Ice::Shared
{
public:

    Object();
    virtual ~Object();

    virtual bool _implements(const std::string&);
    void ____implements(::__Ice::Incoming&);
    virtual const std::string* __ids() = 0;

    typedef void (Object::*__Op)(::__Ice::Incoming&);
    static __Op __ops[];
    static std::string __names[];
    virtual bool __dispatch(::__Ice::Incoming&, const std::string&);

    virtual void __write(::__Ice::Stream*) = 0;
    virtual void __read(::__Ice::Stream*) = 0;
};

}

#endif
