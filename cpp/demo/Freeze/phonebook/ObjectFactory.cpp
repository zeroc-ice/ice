// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <ObjectFactory.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

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
    //
    // Break cyclic object dependencies
    //
    _phoneBook = 0;
    _evictor = 0;
}

void
ContactFactory::initialize(const ObjectAdapterPtr&, const string& identity, const ObjectPtr& servant)
{
    ContactIPtr contact = ContactIPtr::dynamicCast(servant);
    assert(contact);
    contact->setIdentity(identity);
}
