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

    void destroy();

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
    void shutdown();

    NameIdentitiesDict _nameIdentitiesDict;
};

#endif
