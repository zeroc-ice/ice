// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SERVANT_LOCATOR_I_H
#define SERVANT_LOCATOR_I_H

#include <Ice/Ice.h>

namespace Test
{

class ServantLocatorI : public Ice::ServantLocator
{
public:

    ServantLocatorI(const std::string&);
    virtual ~ServantLocatorI();

    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);

protected:

    virtual Ice::ObjectPtr newServantAndCookie(Ice::LocalObjectPtr&) const = 0;
    virtual void checkCookie(const Ice::LocalObjectPtr&) const = 0;
    virtual void throwTestIntfUserException() const = 0;

private:

    void exception(const Ice::Current&);

    const std::string _category;
    bool _deactivated;
    Ice::Int _requestId;
};

};

#endif
