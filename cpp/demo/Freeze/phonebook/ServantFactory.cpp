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
using namespace Ice;
using namespace Freeze;

PhoneBookFactory::PhoneBookFactory(const ObjectAdapterPtr& adapter, const EvictorPtr& evictor) :
    _adapter(adapter),
    _evictor(evictor)
{
}

ObjectPtr
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

ObjectPtr
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

void
ContactFactory::initialize(const ObjectAdapterPtr&, const string& identity, const ObjectPtr& servant)
{
    ContactIPtr contact = ContactIPtr::dynamicCast(servant);
    assert(contact);
    contact->setIdentity(identity);
}
