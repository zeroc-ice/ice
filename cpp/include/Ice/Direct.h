// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_DIRECT_H
#define ICE_DIRECT_H

#include <Ice/ServantLocatorF.h>
#include <Ice/ReferenceF.h>
#include <Ice/ObjectF.h>
#include <Ice/LocalObjectF.h>
#include <Ice/Current.h>

namespace IceInternal
{

class ICE_API Direct : public IceUtil::noncopyable
{
public:

    Direct(const Ice::Current&);
    ~Direct();

    const Ice::ObjectPtr& servant();    

private:

    //
    // Optimization. The current may not be deleted while a
    // stack-allocated Direct still holds it.
    //
    const Ice::Current& _current;

    Ice::ObjectPtr _servant;
    Ice::ServantLocatorPtr _locator;
    Ice::LocalObjectPtr _cookie;
};

}

#endif
