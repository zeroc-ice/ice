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

using namespace std;
using namespace Ice;
using namespace Freeze;

ContactI::ContactI(const PhoneBookIPtr& phoneBook, const EvictorPtr& evictor) :
    _phoneBook(phoneBook),
    _evictor(evictor)
{
}

void
ContactI::setIdentity(const string& ident)
{
    _identity = ident;
}

string
ContactI::getName(const Ice::Current&)
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    return _name;
}

void
ContactI::setName(const string& name, const Ice::Current&)
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    assert(!_identity.empty());
    _phoneBook->move(_identity, _name, name);
    _name = name;
}

string
ContactI::getAddress(const Ice::Current&)
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    return _address;
}

void
ContactI::setAddress(const string& address, const Ice::Current&)
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    _address = address;
}

string
ContactI::getPhone(const Ice::Current&)
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    return _phone;
}

void
ContactI::setPhone(const string& phone, const Ice::Current&)
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    _phone = phone;
}

void
ContactI::destroy(const Ice::Current&)
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

    ContactPrx operator()(const string& ident)
    {
	return ContactPrx::uncheckedCast(_adapter->createProxy(ident));
    }

private:

    ObjectAdapterPtr _adapter;
};

ContactPrx
PhoneBookI::createContact(const Ice::Current&)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock
    
    //
    // Get a new unique identity.
    //
    string ident = getNewIdentity();

    //
    // Create a new Contact Servant.
    //
    ContactIPtr contact = new ContactI(this, _evictor);
    contact->setIdentity(ident);
    
    //
    // Create a new Ice Object in the evictor, using the new identity
    // and the new Servant.
    //
    _evictor->createObject(ident, contact);

    //
    // Add the identity to our name/identities map. The initial name
    // is the empty string.
    //
    Identities identities;
    try
    {
	identities = _nameIdentitiesDict->get("N");
    }
    catch(const DBNotFoundException&)
    {
    }
    identities.push_back(ident);
    _nameIdentitiesDict->put("N", identities);
    
    //
    // Turn the identity into a Proxy and return the Proxy to the
    // caller.
    //
    return IdentityToContact(_adapter)(ident);
}

Contacts
PhoneBookI::findContacts(const string& name, const Ice::Current&)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock
    
    //
    // Lookup all phone book contacts that match a name, and return
    // them to the caller.
    //
    Identities identities;
    try
    {
	identities = _nameIdentitiesDict->get("N" + name);
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
PhoneBookI::setEvictorSize(Int size, const Ice::Current&)
{
    //
    // No synchronization necessary, _evictor is immutable.
    //

    _evictor->setSize(size);
}

void
PhoneBookI::shutdown(const Ice::Current&)
{
    //
    // No synchronization necessary, _adapter is immutable.
    //

    _adapter->getCommunicator()->shutdown();
}

void
PhoneBookI::remove(const string& ident, const string& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    //
    // We do not catch DBNotFoundExceptionPtrE, because it is an
    // application error if name is not found.
    //
    Identities identities  = _nameIdentitiesDict->get("N" + name);
    identities.erase(remove_if(identities.begin(), identities.end(), bind2nd(equal_to<string>(), ident)),
		     identities.end());
    if (identities.empty())
    {
	_nameIdentitiesDict->del("N" + name);
    }
    else
    {
	_nameIdentitiesDict->put("N" + name, identities);
    }
}

void
PhoneBookI::move(const string& ident, const string& oldName, const string& newName)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    //
    // Called by ContactI in case the name has been changed.
    //
    remove(ident, oldName);
    Identities identities;
    try
    {
	identities = _nameIdentitiesDict->get("N" + newName);
    }
    catch(const DBNotFoundException&)
    {
    }
    identities.push_back(ident);
    _nameIdentitiesDict->put("N" + newName, identities);
}

string
PhoneBookI::getNewIdentity()
{
    Ice::Long id;
    Identities ids;
    try
    {
	ids = _nameIdentitiesDict->get("ID");
	assert(ids.size() == 1);
#ifdef WIN32
	id = _atoi64(ids.front().c_str()) + 1;
#else
	id = atoll(ids.front().c_str()) + 1;
#endif
    }
    catch(const DBNotFoundException&)
    {
	id = 0;
    }

    char s[20];
#ifdef WIN32
    sprintf(s, "%I64d", id);
#else
    sprintf(s, "%lld", id);
#endif
    
    ids.clear();
    ids.push_back(s);
    _nameIdentitiesDict->put("ID", ids);

    return string("contact#") + s;
}
