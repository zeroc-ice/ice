// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef SERVANT_FACTORY_H
#define SERVANT_FACTORY_H

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
