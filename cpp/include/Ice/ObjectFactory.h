// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_FACTORY_H
#define ICE_OBJECT_FACTORY_H

#include <Ice/ObjectFactoryF.h>
#include <Ice/StubF.h>
#include <Ice/InstanceF.h>
#include <Ice/Shared.h>

namespace _Ice
{

class ICE_API ObjectFactoryI : public Shared, JTCMutex
{
public:
    
    Ice::Object referenceDataFromString(const std::string&);
    std::string referenceDataToString(const Ice::Object&);

private:

    ObjectFactoryI(const ObjectFactoryI&);
    void operator=(const ObjectFactoryI&);

    ObjectFactoryI(const Instance&);
    virtual ~ObjectFactoryI();
    void destroy();
    friend class InstanceI; // May create and destroy ObjectFactoryIs

    Instance instance_;
};

}

#endif
