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
#include <Ice/InstanceF.h>
#include <Ice/StubF.h>
#include <Ice/Shared.h>

namespace __Ice
{

class Stream;

class ICE_API ObjectFactoryI : public Shared
{
public:
    
    Ice::Object referenceFromString(const std::string&);
    Ice::Object referenceFromStream(Stream*);
    std::string referenceToString(const Ice::Object&);

private:

    ObjectFactoryI(const ObjectFactoryI&);
    void operator=(const ObjectFactoryI&);

    ObjectFactoryI(const Instance&);
    virtual ~ObjectFactoryI();
    friend class InstanceI; // May create and destroy ObjectFactoryIs

    Instance instance_;
};

template<typename T>
void streamWrite(Stream*, const T&);

template<typename T>
void streamRead(Stream*, T&);

template<>
void streamWrite< ::Ice::Object>(Stream*, const ::Ice::Object&);

template<>
void streamRead< ::Ice::Object>(Stream*, ::Ice::Object&);

}

#endif
