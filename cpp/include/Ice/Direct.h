// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

class ICE_API Direct : private IceUtil::noncopyable
{
public:

    Direct(const Ice::Current&);
    
    void destroy();

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
