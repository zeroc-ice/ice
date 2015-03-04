// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef PHONE_BOOK_I_H
#define PHONE_BOOK_I_H

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <PhoneBook.h>
#include <NameIndex.h>
#include <ContactFactory.h>

class PhoneBookI;
typedef IceUtil::Handle<PhoneBookI> PhoneBookIPtr;

class ContactI;
typedef IceUtil::Handle<ContactI> ContactIPtr;

#ifdef __SUNPRO_CC
#   pragma error_messages(off,hidef)
#endif

class ContactI : public Demo::Contact, public IceUtil::AbstractMutexI<IceUtil::Mutex>
{
public:

    ContactI(const ContactFactoryPtr&);

    virtual std::string getName(const Ice::Current&) const;
    virtual void setName(const std::string&, const Ice::Current&);

    virtual std::string getAddress(const Ice::Current&) const;
    virtual void setAddress(const std::string&, const Ice::Current&);

    virtual std::string getPhone(const Ice::Current&) const;
    virtual void setPhone(const std::string&, const Ice::Current&);

    virtual void destroy(const Ice::Current&);

private:

    const ContactFactoryPtr _factory;
};

#ifdef __SUNPRO_CC
#   pragma error_messages(default,hidef)
#endif

class PhoneBookI : public Demo::PhoneBook
{
public: 

    PhoneBookI(const Freeze::EvictorPtr& evictor, const ContactFactoryPtr& factory, const NameIndexPtr& index);

    virtual Demo::ContactPrx createContact(const Ice::Current&);
    virtual Demo::Contacts findContacts(const std::string&, const Ice::Current&) const;
    virtual void setEvictorSize(Ice::Int, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    const Freeze::EvictorPtr _evictor;
    const ContactFactoryPtr _contactFactory;
    NameIndexPtr _index;
};

#endif
