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

class PhoneBookFactory : public Ice::ServantFactory
{
public:

    PhoneBookFactory(const Ice::ObjectAdapterPtr&, const EvictorPtr&);

    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

private:

    Ice::ObjectAdapterPtr _adapter;
    EvictorPtr _evictor;
};

class ContactFactory : public Ice::ServantFactory
{
public:

    ContactFactory(const PhoneBookIPtr&, const EvictorPtr&);

    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

private:

    PhoneBookIPtr _phoneBook;
    EvictorPtr _evictor;
};

#endif
