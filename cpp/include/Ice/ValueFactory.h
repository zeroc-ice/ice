// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_VALUE_FACTORY_H
#define ICE_VALUE_FACTORY_H

#include <Ice/ValueFactoryF.h>
#include <Ice/ObjectF.h>
#include <Ice/Shared.h>

namespace Ice
{

class ICE_API ValueFactoryI : public ::__Ice::Shared
{
public:

    ValueFactoryI() { }
    virtual std::vector<std::string> ids() = 0;
    virtual Ice::Object_iptr create(const std::string&) = 0;
};

}

#endif
