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

ServantFactory::ServantFactory(const PhoneBookIPtr& phoneBook) :
    _phoneBook(phoneBook)
{
}

Ice::ObjectPtr
ServantFactory::create(const string& type)
{
    if (type == "::Entry")
    {
	return new EntryI(_phoneBook);
    }

    assert(false);
    return 0;
}
