// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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
    ["nonmutating"] string getName();
    void setName(string name)
	throws DatabaseException;

    ["nonmutating"] string getAddress();
    void setAddress(string address);

    ["nonmutating"] string getPhone();
    void setPhone(string phone);

    //
    // Yes, destroy() is nonmutating. It doesn't change the state of
    // the Contact. It removes the Contact completely, but doesn't
    // touch state.
    //
    ["nonmutating"] void destroy()
	throws DatabaseException;

    string _name;
    string _address;
    string _phone;
};

sequence<Contact*> Contacts;
sequence<Ice::Identity> Identities; // Needed for slice2freeze

interface PhoneBook
{
    Contact* createContact()
	throws DatabaseException;
    ["nonmutating"] Contacts findContacts(string name)
	throws DatabaseException;

    void setEvictorSize(int size)
	throws DatabaseException;
    ["nonmutating"] void shutdown();
};

#endif
