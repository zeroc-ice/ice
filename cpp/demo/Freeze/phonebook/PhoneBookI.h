// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef PHONE_BOOK_I_H
#define PHONE_BOOK_I_H

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Freeze/Freeze.h>
#include <PhoneBook.h>
#include <NameIndex.h>

class PhoneBookI;
typedef IceUtil::Handle<PhoneBookI> PhoneBookIPtr;

class ContactI;
typedef IceUtil::Handle<ContactI> ContactIPtr;

class ContactI : public Contact, 
		 public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
{
public:

    ContactI(const Freeze::EvictorPtr&);

    virtual std::string getName(const Ice::Current&) const;
    virtual void setName(const std::string&, const Ice::Current&);

    virtual std::string getAddress(const Ice::Current&) const;
    virtual void setAddress(const std::string&, const Ice::Current&);

    virtual std::string getPhone(const Ice::Current&) const;
    virtual void setPhone(const std::string&, const Ice::Current&);

    virtual void destroy(const Ice::Current&);

private:

    Freeze::EvictorPtr _evictor;
};

class PhoneBookI : public PhoneBook
{
public: 

    PhoneBookI(const Freeze::EvictorPtr& evictor, const NameIndexPtr& index);

    virtual ContactPrx createContact(const Ice::Current&);
    virtual Contacts findContacts(const std::string&, const Ice::Current&) const;
    virtual void setEvictorSize(Ice::Int, const Ice::Current&);
    virtual void shutdown(const Ice::Current&) const;

private:

    Freeze::EvictorPtr _evictor;
    NameIndexPtr _index;
};

#endif
