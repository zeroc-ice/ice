// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <PhoneBookI.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace Freeze;

ContactI::ContactI(const PhoneBookIPtr& phoneBook, const EvictorPtr& evictor) :
    _phoneBook(phoneBook),
    _evictor(evictor)
{
}

void
ContactI::setIdentity(const string& identity)
{
    _identity = identity;
}

string
ContactI::getName()
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    return _name;
}

void
ContactI::setName(const string& name)
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    assert(!_identity.empty());
    _phoneBook->move(_identity, _name, name);
    _name = name;
}

string
ContactI::getAddress()
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    return _address;
}

void
ContactI::setAddress(const string& address)
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    _address = address;
}

string
ContactI::getPhone()
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    return _phone;
}

void
ContactI::setPhone(const string& phone)
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    _phone = phone;
}

void
ContactI::destroy()
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    assert(!_identity.empty());
    _phoneBook->remove(_identity, _name);
    _evictor->destroyObject(_identity);
}

PhoneBookI::PhoneBookI(const ObjectAdapterPtr& adapter, const DBPtr& db, const EvictorPtr& evictor) :
    _adapter(adapter),
    _db(db),
    _evictor(evictor),
    _nameIdentitiesDict(new NameIdentitiesDict(db))
{
}

class IdentityToContact
{
public:

    IdentityToContact(const ObjectAdapterPtr& adapter) :
	_adapter(adapter)
    {
    }

    ContactPrx operator()(const string& identity)
    {
	return ContactPrx::uncheckedCast(_adapter->createProxy(identity));
    }

private:

    ObjectAdapterPtr _adapter;
};

ContactPrx
PhoneBookI::createContact()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock
    
    //
    // Get a unique ID
    //
    string identity = "phonebook.contact#";
#ifdef WIN32 // COMPILERBUG
    char s[20];
    sprintf(s, "%I64d", _nextContactIdentity++);
    identity += s;
#else
    ostringstream s;
    s << _nextContactIdentity++;
    identity += s.str();
#endif
    
    //
    // Create a new contact Servant.
    //
    ContactIPtr contact = new ContactI(this, _evictor);
    contact->setIdentity(identity);

    //
    // Create a new Ice Object in the evictor, using the new identity
    // and the new Servant.
    //
    _evictor->createObject(identity, contact);

    //
    // Add the identity to our name/identities map. The initial name
    // is the empty string.
    //
    Identities identities;
    try
    {
	identities = _nameIdentitiesDict->get("");
    }
    catch(const DBNotFoundException&)
    {
    }
    identities.push_back(identity);
    _nameIdentitiesDict->put("", identities);
    
    //
    // Turn the identity into a Proxy and return the Proxy to the
    // caller.
    //
    return IdentityToContact(_adapter)(identity);
}

Contacts
PhoneBookI::findContacts(const string& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock
    
    //
    // Lookup all phone book contacts that match a name, and return
    // them to the caller.
    //
    Identities identities;
    try
    {
	identities = _nameIdentitiesDict->get(name);
    }
    catch(const DBNotFoundException&)
    {
    }

    Contacts contacts;
    contacts.reserve(identities.size());
    transform(identities.begin(), identities.end(), back_inserter(contacts), IdentityToContact(_adapter));
    return contacts;
}

void
PhoneBookI::shutdown()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    _adapter->getCommunicator()->shutdown();
}

void
PhoneBookI::remove(const string& identity, const string& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    //
    // We do not catch DBNotFoundException, because it is an
    // application error if name is not found.
    //
    Identities identities  = _nameIdentitiesDict->get(name);
    identities.erase(remove_if(identities.begin(), identities.end(), bind2nd(equal_to<string>(), identity)),
		     identities.end());
    _nameIdentitiesDict->put(name, identities);
}

void
PhoneBookI::move(const string& identity, const string& oldName, const string& newName)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    //
    // Called by ContactI in case the name has been changed.
    //
    remove(identity, oldName);
    Identities identities;
    try
    {
	identities = _nameIdentitiesDict->get(newName);
    }
    catch(const DBNotFoundException&)
    {
    }
    identities.push_back(identity);
    _nameIdentitiesDict->put(newName, identities);
}
