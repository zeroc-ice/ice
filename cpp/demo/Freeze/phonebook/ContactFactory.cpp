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

#include <ContactFactory.h>

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
ContactFactory::initialize(const ObjectAdapterPtr&, const Identity& ident, const ObjectPtr& servant)
{
    ContactIPtr contact = ContactIPtr::dynamicCast(servant);
    assert(contact);
    contact->setIdentity(ident);
}
