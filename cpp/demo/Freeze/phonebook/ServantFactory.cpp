// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <ServantFactory.h>

using namespace std;

PhoneBookFactory::PhoneBookFactory(const Ice::ObjectAdapterPtr& adapter, const EvictorPtr& evictor) :
    _adapter(adapter),
    _evictor(evictor)
{
}

Ice::ObjectPtr
PhoneBookFactory::create(const string& type)
{
    assert(type == "::PhoneBook");
    return new PhoneBookI(_adapter, _evictor);
}

void
PhoneBookFactory::destroy()
{
    // Nothing to do
}

ContactFactory::ContactFactory(const PhoneBookIPtr& phoneBook, const EvictorPtr& evictor) :
    _phoneBook(phoneBook),
    _evictor(evictor)
{
}

Ice::ObjectPtr
ContactFactory::create(const string& type)
{
    assert(type == "::Contact");
    return new ContactI(_phoneBook, _evictor);
}

void
ContactFactory::destroy()
{
    // Nothing to do
}
