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

class ServantFactory : public Ice::ServantFactory
{
public:

    ServantFactory(const PhoneBookIPtr&);

    virtual Ice::ObjectPtr create(const std::string&);

private:

    PhoneBookIPtr _phoneBook;
};

#endif
