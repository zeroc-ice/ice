// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_DIRECT_H
#define ICE_DIRECT_H

#include <Ice/ServantLocatorF.h>
#include <Ice/ReferenceF.h>
#include <Ice/Object.h>
#include <Ice/LocalObjectF.h>
#include <Ice/Current.h>

#include <IceUtil/UniquePtr.h>

namespace IceInternal
{

class ICE_API Direct : public Ice::Request, private IceUtil::noncopyable
{
public:

    Direct(const Ice::Current&);
    void destroy();

    const Ice::ObjectPtr& getServant();

    virtual bool isCollocated();
    virtual const Ice::Current& getCurrent();
    virtual Ice::DispatchStatus run(Ice::Object*) = 0;

    void throwUserException();

protected:

    //
    // Optimization. The current may not be deleted while a
    // stack-allocated Direct still holds it.
    //
    const Ice::Current& _current;

    void setUserException(const Ice::UserException&);

private:

    Ice::ObjectPtr _servant;
    Ice::ServantLocatorPtr _locator;
    Ice::LocalObjectPtr _cookie;
    IceUtil::UniquePtr<Ice::UserException> _userException;
};

}

#endif
