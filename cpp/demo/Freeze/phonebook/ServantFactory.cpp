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

ServantFactory::ServantFactory(const PhoneBookIPtr& phoneBook, const EvictorPtr& evictor) :
    _phoneBook(phoneBook),
    _evictor(evictor)
{
}

Ice::ObjectPtr
ServantFactory::create(const string& type)
{
    assert(type == "::Entry");
    return new EntryI(_phoneBook, _evictor);
}
