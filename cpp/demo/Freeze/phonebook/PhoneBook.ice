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
sequence<string> Identities; // Needed for slice2freeze

interface PhoneBook
{
    Contact* createContact();
    Contacts findContacts(string name);
    void shutdown();
};

#endif
