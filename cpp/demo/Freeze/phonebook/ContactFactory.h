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

#include <PhoneBookI.h>

class ContactFactory : virtual public Ice::ObjectFactory, virtual public Freeze::ServantInitializer
{
public:

    ContactFactory(const PhoneBookIPtr&, const Freeze::EvictorPtr&);

    //
    // Operations from ObjectFactory
    //
    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

    //
    // Operations from ServantInitializer
    //
    virtual void initialize(const Ice::ObjectAdapterPtr&, const Ice::Identity&, const Ice::ObjectPtr&);

private:

    PhoneBookIPtr _phoneBook;
    Freeze::EvictorPtr _evictor;
};

#endif
