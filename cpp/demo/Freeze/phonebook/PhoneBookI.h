// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef PHONE_BOOK_I_H
#define PHONE_BOOK_I_H

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <PhoneBook.h>

class PhoneBookI;
typedef IceUtil::Handle<PhoneBookI> PhoneBookIPtr;

class EntryI : public Entry, public JTCMutex
{
public:

    EntryI(const std::string& identity, const PhoneBookIPtr&);
    ~EntryI();

    virtual std::wstring getName();
    virtual void setName(const std::wstring&);

    virtual std::wstring getAddress();
    virtual void setAddress(const std::wstring&);

    virtual std::string getNumber();
    virtual void setNumber(const std::string&);

    virtual void destroy();

private:

    std::string _identity;
    PhoneBookIPtr _phoneBook;
};

class PhoneBookI : public PhoneBook, public JTCRecursiveMutex
{
public: 

    PhoneBookI(const Ice::ObjectAdapterPtr&);
    ~PhoneBookI();

    virtual EntryPrx createEntry();
    virtual Entries findEntries(const std::wstring&);
    virtual Names listNames();
    
    void add(const std::string&, const std::wstring&);
    void remove(const std::string&, const std::wstring&);
    void move(const std::string&, const std::wstring&, const std::wstring&);

private:

    Ice::ObjectAdapterPtr _adapter;
    Ice::Long _nextEntryIdentity;
};

#endif
