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
ContactI::setIdentity(const Identity& ident)
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
    assert(!_identity.name.empty());
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
    assert(!_identity.name.empty());
    _phoneBook->remove(_identity, _name);
    _evictor->destroyObject(_identity);
}

PhoneBookI::PhoneBookI(const ObjectAdapterPtr& adapter, const DBPtr& db, const EvictorPtr& evictor) :
    _adapter(adapter),
    _db(db),
    _evictor(evictor),
    _nameIdentitiesDict(db)
{
}

class IdentityToContact
{
public:

    IdentityToContact(const ObjectAdapterPtr& adapter) :
	_adapter(adapter)
    {
    }

    ContactPrx operator()(const Identity& ident)
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
    Identity ident = getNewIdentity();

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
    // is the empty string. See the comment in getNewIdentity why the
    // prefix "N" is needed.
    //
    NameIdentitiesDict::iterator p = _nameIdentitiesDict.find("N");
    Identities identities;
    if (p != _nameIdentitiesDict.end())
    {
	identities = p->second;
    }

    identities.push_back(ident);
    _nameIdentitiesDict.insert(NameIdentitiesDict::value_type("N", identities));
    
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
    // them to the caller. See the comment in getNewIdentity why the
    // prefix "N" is needed.
    //
    NameIdentitiesDict::iterator p = _nameIdentitiesDict.find("N" + name);
    Identities identities;
    if (p !=  _nameIdentitiesDict.end())
    {
	identities = p->second;
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
PhoneBookI::remove(const Identity& ident, const string& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    //
    // See the comment in getNewIdentity why the prefix "N" is needed.
    //
    NameIdentitiesDict::iterator p = _nameIdentitiesDict.find("N" + name);

    //
    // If the name isn't found then this is an application
    // error. Throw a DBNotFoundException.
    //
    if (p == _nameIdentitiesDict.end())
    {
	throw DBNotFoundException();
    }

    Identities identities  = p->second;
    identities.erase(remove_if(identities.begin(), identities.end(), bind2nd(equal_to<Ice::Identity>(), ident)),
		     identities.end());

    if (identities.empty())
    {
	_nameIdentitiesDict.erase(p);
    }
    else
    {
	//
	// See the comment in getNewIdentity why the prefix "N" is
	// needed.
	//
	_nameIdentitiesDict.insert(NameIdentitiesDict::value_type("N" + name, identities));
    }
}

void
PhoneBookI::move(const Identity& ident, const string& oldName, const string& newName)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    //
    // Called by ContactI in case the name has been changed. See the
    // comment in getNewIdentity why the prefix "N" is needed.
    //
    remove(ident, oldName);
    NameIdentitiesDict::iterator p = _nameIdentitiesDict.find("N" + newName);
    Identities identities;
    if (p != _nameIdentitiesDict.end())
    {
	identities = p->second;
    }
    identities.push_back(ident);
    _nameIdentitiesDict.insert(NameIdentitiesDict::value_type("N" + newName, identities));
}

Identity
PhoneBookI::getNewIdentity()
{
    //
    // This code is a bit of a hack. It stores the last identity that
    // has been used (or the name component thereof, to be more
    // precise) in the _nameIdentitiesDict, with the special prefix
    // "ID". Because of this, all "real" names need to be prefixed
    // with "N", so that there is no potential for a name clash.
    //

    Ice::Long n;
    Identities ids;
    NameIdentitiesDict::iterator p = _nameIdentitiesDict.find("ID");
    if (p == _nameIdentitiesDict.end())
    {
	n = 0;
    }
    else
    {
	ids = p->second;
	assert(ids.size() == 1);
#ifdef WIN32
	n = _atoi64(ids.front().name.c_str()) + 1;
#else
	n = atoll(ids.front().name.c_str()) + 1;
#endif
    }

    char s[20];
#ifdef WIN32
    sprintf(s, "%I64d", n);
#else
    sprintf(s, "%lld", n);
#endif
    
    Identity id;

    id.name = s;
    ids.clear();
    ids.push_back(id);

    _nameIdentitiesDict.insert(NameIdentitiesDict::value_type("ID", ids));

    id.name = s;
    id.category = "contact";
    return id;
}
