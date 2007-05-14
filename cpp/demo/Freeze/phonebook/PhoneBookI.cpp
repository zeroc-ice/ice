// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <PhoneBookI.h>

using namespace std;
using namespace Demo;

ContactI::ContactI(const ContactFactoryPtr& contactFactory) :
    _factory(contactFactory)
{
}

string
ContactI::getName(const Ice::Current&) const
{
    IceUtil::Mutex::Lock lock(*this);
    return name;
}

void
ContactI::setName(const string& newName, const Ice::Current&)
{
    IceUtil::Mutex::Lock lock(*this);
    name = newName;
}

string
ContactI::getAddress(const Ice::Current&) const
{
    IceUtil::Mutex::Lock lock(*this);
    return address;
}

void
ContactI::setAddress(const string& newAddress, const Ice::Current&)
{
    IceUtil::Mutex::Lock lock(*this);
    address = newAddress;
}

string
ContactI::getPhone(const Ice::Current&) const
{
    IceUtil::Mutex::Lock lock(*this);
    return phone;
}

void
ContactI::setPhone(const string& newPhone, const Ice::Current&)
{
    IceUtil::Mutex::Lock lock(*this);
    phone = newPhone;
}

void
ContactI::destroy(const Ice::Current& c)
{
    try
    {
        _factory->getEvictor()->remove(c.id);
    }
    catch(const Freeze::DatabaseException& ex)
    {
        DatabaseException e;
        e.message = ex.message;
        throw e;
    }
}

PhoneBookI::PhoneBookI(const Freeze::EvictorPtr& evictor, const ContactFactoryPtr& contactFactory,
                       const NameIndexPtr& index) :
    _evictor(evictor),
    _contactFactory(contactFactory),
    _index(index)
{
}

class IdentityToContact
{
public:

    IdentityToContact(const Ice::ObjectAdapterPtr& adapter) :
        _adapter(adapter)
    {
    }

        ContactPrx operator()(const Ice::Identity& ident)
    {
        return ContactPrx::uncheckedCast(_adapter->createProxy(ident));
    }

private:

    const Ice::ObjectAdapterPtr _adapter;
};

ContactPrx
PhoneBookI::createContact(const Ice::Current& c)
{
    //
    // Get a new unique identity.
    //
    Ice::Identity ident;
    ident.name = IceUtil::generateUUID();
    ident.category = "contact";

    //
    // Create a new Contact Servant.
    //
    ContactIPtr contact = new ContactI(_contactFactory);
    
    //
    // Create a new Ice Object in the evictor, using the new identity
    // and the new Servant.
    //
    _evictor->add(contact, ident);

    
    //
    // Turn the identity into a Proxy and return the Proxy to the
    // caller.
    //
    return IdentityToContact(c.adapter)(ident);
}

Contacts
PhoneBookI::findContacts(const string& name, const Ice::Current& c) const
{
    try
    {
        vector<Ice::Identity> identities = _index->find(name);
        
        Contacts contacts;
        contacts.reserve(identities.size());
        transform(identities.begin(), identities.end(), back_inserter(contacts), IdentityToContact(c.adapter));

        return contacts;
    }
    catch(const Freeze::DatabaseException& ex)
    {
        DatabaseException e;
        e.message = ex.message;
        throw e;
    }
}

void
PhoneBookI::setEvictorSize(Ice::Int size, const Ice::Current&)
{
    _evictor->setSize(size);
}

void
PhoneBookI::shutdown(const Ice::Current& c)
{
    c.adapter->getCommunicator()->shutdown();
}

