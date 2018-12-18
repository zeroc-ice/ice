// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
#ifdef ICE_CPP11_MAPPING
    virtual std::shared_ptr<Ice::Object> locate(const Ice::Current&, std::shared_ptr<void>&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const std::shared_ptr<void>&);
#else
    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
#endif
    virtual void deactivate(const std::string&);

protected:

#ifdef ICE_CPP11_MAPPING
    virtual Ice::ObjectPtr newServantAndCookie(std::shared_ptr<void>&) const = 0;
    virtual void checkCookie(const std::shared_ptr<void>&) const = 0;
#else
    virtual Ice::ObjectPtr newServantAndCookie(Ice::LocalObjectPtr&) const = 0;
    virtual void checkCookie(const Ice::LocalObjectPtr&) const = 0;
#endif
    virtual void throwTestIntfUserException() const = 0;

private:

    void exception(const Ice::Current&);

    const std::string _category;
    bool _deactivated;
    Ice::Int _requestId;
};

};

#endif
