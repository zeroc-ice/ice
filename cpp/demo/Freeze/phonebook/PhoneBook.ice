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

class Contact
{
    nonmutating string getName();
    void setName(string name);

    nonmutating string getAddress();
    void setAddress(string address);

    nonmutating string getPhone();
    void setPhone(string phone);

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
sequence<string> Identities;
dictionary<string, Identities> NameIdentitiesDict;
sequence<string> Names;

class PhoneBook
{
    Contact* createContact();

    nonmutating Contacts findContacts(string name);

    nonmutating Names getAllNames();

    //
    // shutdown() is nonmutating. It doesn't change any state, it only
    // shuts down the application.
    //
    nonmutating void shutdown();

    long _nextContactIdentity;
    NameIdentitiesDict _nameIdentitiesDict;
};

#endif
