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
#include <Ice/ReferenceF.h>
#include <Ice/ObjectF.h>
#include <Ice/LocalObjectF.h>

namespace IceInternal
{

class ICE_API Direct : public ::IceUtil::noncopyable
{
public:

    Direct(const ::Ice::ObjectAdapterPtr&, const ReferencePtr&, const char*);
    ~Direct();

    const ::Ice::ObjectPtr& servant();    

private:

    const ::Ice::ObjectAdapterPtr& _adapter;
    const ReferencePtr& _reference;
    const char* _operation;
    ::Ice::ObjectPtr _servant;
    ::Ice::ServantLocatorPtr _locator;
    ::Ice::LocalObjectPtr _cookie;
};

}

#endif
