// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef PHONE_BOOK_ICE
#define PHONE_BOOK_ICE

#include <Ice/Identity.ice>

exception DatabaseException
{
    string message;
};

class Contact
{
    nonmutating string getName();
    idempotent void setName(string name)
	throws DatabaseException;

    nonmutating string getAddress();
    idempotent void setAddress(string address);

    nonmutating string getPhone();
    idempotent void setPhone(string phone);

    void destroy()
	throws DatabaseException;

    string name;
    string address;
    string phone;
};

sequence<Contact*> Contacts;
sequence<Ice::Identity> Identities; // Needed for slice2freeze

interface PhoneBook
{
    Contact* createContact()
	throws DatabaseException;
    nonmutating Contacts findContacts(string name)
	throws DatabaseException;

    idempotent void setEvictorSize(int size)
	throws DatabaseException;
    nonmutating void shutdown();
};

#endif
