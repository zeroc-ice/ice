// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <ServantInitializer.h>

using namespace std;
using namespace Ice;

ContactInitializer::ContactInitializer(const PhoneBookIPtr& phoneBook) :
    _phoneBook(phoneBook)
{
}

void
ContactInitializer::initialize(const ObjectAdapterPtr&, const string& identity, const ObjectPtr& servant)
{
    ContactIPtr entry = ContactIPtr::dynamicCast(servant);
    assert(entry);
    entry->setIdentity(identity);
}
