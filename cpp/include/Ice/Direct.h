// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_DIRECT_H
#define ICE_DIRECT_H

#include <Ice/ObjectAdapterF.h>
#include <Ice/ServantLocatorF.h>
#include <Ice/ReferenceF.h>
#include <Ice/ObjectF.h>
#include <Ice/LocalObjectF.h>
#include <Ice/Current.h>

namespace IceInternal
{

class ICE_API Direct : public ::IceUtil::noncopyable
{
public:

    Direct(const ::Ice::ObjectAdapterPtr&, const ::Ice::Current&);
    ~Direct();

    const ::Ice::ObjectPtr& facetServant();    

private:

    const ::Ice::ObjectAdapterPtr& _adapter;
    ::Ice::Current _current;
    ::Ice::ObjectPtr _servant;
    ::Ice::ObjectPtr _facetServant;
    ::Ice::ServantLocatorPtr _locator;
    ::Ice::LocalObjectPtr _cookie;
};

}

#endif
