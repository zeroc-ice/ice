// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef PHONE_BOOK_ICE
#define PHONE_BOOK_ICE

#include <Freeze/DBException.ice>

class Contact
{
    nonmutating string getName() throws Freeze::DBException;
    void setName(string name) throws Freeze::DBException;

    nonmutating string getAddress() throws Freeze::DBException;
    void setAddress(string address) throws Freeze::DBException;

    nonmutating string getPhone() throws Freeze::DBException;
    void setPhone(string phone) throws Freeze::DBException;

    //
    // Yes, destroy() is nonmutating. It doesn't change the state of
    // the Contact. It removes the Contact completely, but doesn't
    // touch state.
    //
    nonmutating void destroy();

    string _name;
    string _address;
    string _phone;
};

sequence<Contact*> Contacts;
sequence<string> Identities; // Needed for slice2freeze

interface PhoneBook
{
    Contact* createContact() throws Freeze::DBException;
    nonmutating Contacts findContacts(string name) throws Freeze::DBException;
    void setEvictorSize(int size) throws Freeze::DBException;
    nonmutating void shutdown() throws Freeze::DBException;
};

#endif
