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

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Freeze/Freeze.h>
#include <PhoneBook.h>
#include <NameIdentitiesDict.h>

class PhoneBookI;
typedef IceUtil::Handle<PhoneBookI> PhoneBookIPtr;

class ContactI;
typedef IceUtil::Handle<ContactI> ContactIPtr;

class ContactI : public Contact, public IceUtil::RWRecMutex
{
public:

    ContactI(const PhoneBookIPtr&, const Freeze::EvictorPtr&);

    void setIdentity(const Ice::Identity&);

    virtual std::string getName(const Ice::Current&);
    virtual void setName(const std::string&, const Ice::Current&);

    virtual std::string getAddress(const Ice::Current&);
    virtual void setAddress(const std::string&, const Ice::Current&);

    virtual std::string getPhone(const Ice::Current&);
    virtual void setPhone(const std::string&, const Ice::Current&);

    virtual void destroy(const Ice::Current&);

private:

    PhoneBookIPtr _phoneBook;
    Freeze::EvictorPtr _evictor;
    Ice::Identity _identity;
};

class PhoneBookI : public PhoneBook, public IceUtil::RWRecMutex
{
public: 

    PhoneBookI(const Ice::ObjectAdapterPtr&, const Freeze::DBPtr&, const Freeze::EvictorPtr&);

    virtual ContactPrx createContact(const Ice::Current&);
    virtual Contacts findContacts(const std::string&, const Ice::Current&);
    virtual void setEvictorSize(Ice::Int, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
    
    void remove(const Ice::Identity&, const std::string&);
    void move(const Ice::Identity&, const std::string&, const std::string&);
    Ice::Identity getNewIdentity();

private:

    void removeI(const Ice::Identity&, const std::string&);

    Ice::ObjectAdapterPtr _adapter;
    Freeze::DBPtr _db;
    Freeze::EvictorPtr _evictor;
    NameIdentitiesDict _nameIdentitiesDict;
};

#endif
