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

class Entry
{
    string getName();
    void setName(string name);

    string getAddress();
    void setAddress(string address);

    string getPhone();
    void setPhone(string phone);

    void destroy();

    string _name;
    string _address;
    string _phone;
};

sequence<Entry*> Entries;
sequence<string> Identities;
dictionary<string, Identities> NameIdentitiesDict;
sequence<string> Names;

class PhoneBook
{
    Entry* createEntry();
    Entries findEntries(string name);
    Names getAllNames();

    NameIdentitiesDict _nameIdentitiesDict;
};

#endif
