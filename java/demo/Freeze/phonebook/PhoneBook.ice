// **********************************************************************
//
// Copyright (c) 2001
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
    void setName(string name) throws DatabaseException;

    nonmutating string getAddress();
    void setAddress(string address);

    nonmutating string getPhone();
    void setPhone(string phone);

    void destroy() throws DatabaseException;

    string name;
    string address;
    string phone;
};

sequence<Contact*> Contacts;
sequence<Ice::Identity> Identities; // Needed for slice2freeze

interface PhoneBook
{
    Contact* createContact() throws DatabaseException;
    nonmutating Contacts findContacts(string name) throws DatabaseException;
    void setEvictorSize(int size) throws DatabaseException;
    idempotent void shutdown();
};

#endif
