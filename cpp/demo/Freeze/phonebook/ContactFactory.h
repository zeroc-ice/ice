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

#ifndef CONTACT_FACTORY_H
#define CONTACT_FACTORY_H

#include <Freeze/Freeze.h>

class ContactFactory : virtual public Ice::ObjectFactory
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
    
    const Freeze::EvictorPtr&
    getEvictor() const;
    

private:

    Freeze::EvictorPtr _evictor;
};

typedef IceUtil::Handle<ContactFactory> ContactFactoryPtr;

#endif
