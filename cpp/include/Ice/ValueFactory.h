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

class ICE_API ValueFactory : public ::__Ice::Shared
{
public:

    ValueFactory() { }

    virtual Ice::Object_ptr create(const std::string&) = 0;
};

}

#endif
