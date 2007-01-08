// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CONTACT_FACTORY_H
#define CONTACT_FACTORY_H

#include <Freeze/Freeze.h>

class ContactFactory : public Ice::ObjectFactory
{
public:

    ContactFactory();

    //
    // Operations from ObjectFactory
    //
    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

    void
    setEvictor(const Freeze::EvictorPtr&);
    
    Freeze::EvictorPtr
    getEvictor() const;

private:

    Freeze::EvictorPtr _evictor;
};

typedef IceUtil::Handle<ContactFactory> ContactFactoryPtr;

#endif
