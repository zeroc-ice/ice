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
    wstring getName();
    void setName(wstring name);

    wstring getAddress();
    void setAddress(wstring address);

    string getNumber();
    void setNumber(string number);

    void destroy();

    wstring _name;
    wstring _address;
    string _number;
};

sequence<Entry*> Entries;
sequence<string> Identities;
dictionary<wstring, Identities> NameIdentitiesDict;
sequence<wstring> Names;

class PhoneBook
{
    Entry* createEntry();
    Entries findEntries(wstring name);
    Names listNames();

    NameIdentitiesDict _nameIdentitiesDict;
};

#endif
