// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef SERVANT_INITIALIZER_H
#define SERVANT_INITIALIZER_H

#include <PhoneBookI.h>

class ContactInitializer : public Freeze::ServantInitializer
{
public:

    ContactInitializer(const PhoneBookIPtr&);

    virtual void initialize(const Ice::ObjectAdapterPtr&, const std::string&, const Ice::ObjectPtr&);

private:

    PhoneBookIPtr _phoneBook;
};

#endif
