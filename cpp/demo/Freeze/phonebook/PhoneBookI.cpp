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

#include <PhoneBookI.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

ContactI::ContactI(const PhoneBookIPtr& phoneBook, const Freeze::EvictorPtr& evictor) :
    _phoneBook(phoneBook),
    _evictor(evictor),
    _destroyed(false)
{
}

void
ContactI::setIdentity(const Identity& ident)
{
    _identity = ident;
}

string
ContactI::getName(const Ice::Current&) const
{
    IceUtil::RWRecMutex::RLock sync(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    return name;
}

void
ContactI::setName(const string& newName, const Ice::Current&)
{
    IceUtil::RWRecMutex::WLock sync(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    assert(!_identity.name.empty());
    _phoneBook->move(_identity, name, newName);
    name = newName;
}

string
ContactI::getAddress(const Ice::Current&) const
{
    IceUtil::RWRecMutex::RLock sync(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    return address;
}

void
ContactI::setAddress(const string& newAddress, const Ice::Current&)
{
    IceUtil::RWRecMutex::WLock sync(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    address = newAddress;
}

string
ContactI::getPhone(const Ice::Current&) const
{
    IceUtil::RWRecMutex::RLock sync(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    return phone;
}

void
ContactI::setPhone(const string& newPhone, const Ice::Current&)
{
    IceUtil::RWRecMutex::WLock sync(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    phone = newPhone;
}

void
ContactI::destroy(const Ice::Current&)
{
    IceUtil::RWRecMutex::RLock sync(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    _destroyed = true;

    try
    {
	assert(!_identity.name.empty());
	_phoneBook->remove(_identity, name);

	//
	// This can throw EvictorDeactivatedException (which indicates
	// an internal error). The exception is currently ignored.
	//
	_evictor->destroyObject(_identity);
    }
    catch(const Freeze::DBNotFoundException&)
    {
	//
	// Raised by remove. Ignore.
	//
    }
    catch(const Freeze::DBException& ex)
    {
	DatabaseException e;
	e.message = ex.message;
	throw e;
    }
}

PhoneBookI::PhoneBookI(const Ice::CommunicatorPtr& communicator,
		       const std::string& envName, const std::string& dbName,
		       const Freeze::EvictorPtr& evictor) :
    _evictor(evictor),
    _nameIdentitiesDict(communicator, envName, dbName)
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
PhoneBookI::createContact(const Ice::Current& c)
{
    IceUtil::RWRecMutex::WLock sync(*this);
    
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
    // This can throw EvictorDeactivatedException (which indicates
    // an internal error). The exception is currently ignored.
    //
    _evictor->createObject(ident, contact);

    //
    // Add the identity to our name/identities map. The initial name
    // is the empty string. See the comment in getNewIdentity why the
    // prefix "N" is needed.
    //
    try
    {
	NameIdentitiesDict::iterator p = _nameIdentitiesDict.find("N");
	Identities identities;
	if(p != _nameIdentitiesDict.end())
	{
	    identities = p->second;
	}
	
	identities.push_back(ident);
	_nameIdentitiesDict.put(NameIdentitiesDict::value_type(string("N"), identities));
	
	//
	// Turn the identity into a Proxy and return the Proxy to the
	// caller.
	//
	return IdentityToContact(c.adapter)(ident);
    }
    catch(const Freeze::DBException& ex)
    {
	DatabaseException e;
	e.message = ex.message;
	throw e;
    }
}

Contacts
PhoneBookI::findContacts(const string& name, const Ice::Current& c) const
{
    IceUtil::RWRecMutex::RLock sync(*this);
    
    try
    {
	//
	// Lookup all phone book contacts that match a name, and
	// return them to the caller. See the comment in
	// getNewIdentity why the prefix "N" is needed.
	//
	NameIdentitiesDict::const_iterator p = _nameIdentitiesDict.find("N" + name);
	Identities identities;
	if(p !=  _nameIdentitiesDict.end())
	{
	    identities = p->second;
	}

	Contacts contacts;
	contacts.reserve(identities.size());
	transform(identities.begin(), identities.end(), back_inserter(contacts), IdentityToContact(c.adapter));

	return contacts;
    }
    catch(const Freeze::DBException& ex)
    {
	DatabaseException e;
	e.message = ex.message;
	throw e;
    }
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
PhoneBookI::shutdown(const Ice::Current& c) const
{
    c.adapter->getCommunicator()->shutdown();
}

void
PhoneBookI::remove(const Identity& ident, const string& name)
{
    IceUtil::RWRecMutex::WLock sync(*this);

    try
    {
	removeI(ident, name);
    }
    catch(const Freeze::DBException& ex)
    {
	DatabaseException e;
	e.message = ex.message;
	throw e;
    }
}

void
PhoneBookI::move(const Identity& ident, const string& oldName, const string& newName)
{
    IceUtil::RWRecMutex::WLock sync(*this);

    try
    {
	//
	// Called by ContactI in case the name has been changed. See
	// the comment in getNewIdentity why the prefix "N" is needed.
	//
	removeI(ident, oldName);
	NameIdentitiesDict::iterator p = _nameIdentitiesDict.find("N" + newName);
	Identities identities;
	if(p != _nameIdentitiesDict.end())
	{
	    identities = p->second;
	}
	identities.push_back(ident);
	_nameIdentitiesDict.put(NameIdentitiesDict::value_type("N" + newName, identities));
    }
    catch(const Freeze::DBNotFoundException&)
    {
	//
	// Raised by remove. This should only happen under very rare
	// circumstances if destroy() had gotten to the object prior
	// to the setName() operation being dispatched. Ignore the
	// exception.
	//
    }
    catch(const Freeze::DBException& ex)
    {
	DatabaseException e;
	e.message = ex.message;
	throw e;
    }
}

Identity
PhoneBookI::getNewIdentity()
{
    try
    {
	//
	// This code is a bit of a hack. It stores the last identity
	// that has been used (or the name component thereof, to be
	// more precise) in the _nameIdentitiesDict, with the special
	// prefix "ID". Because of this, all "real" names need to be
	// prefixed with "N", so that there is no potential for a name
	// clash.
	//

	Ice::Long n;
	Identities ids;
	{
	    NameIdentitiesDict::iterator p = _nameIdentitiesDict.find("ID");
	    if(p == _nameIdentitiesDict.end())
	    {
		n = 0;
	    }
	    else
	    {
		ids = p->second;
		assert(ids.size() == 1);
		
		string::size_type sz;
		bool rc = IceUtil::stringToInt64(ids.front().name, n, sz);
		assert(rc);
		n += 1;
	    }
	}

	char s[20];

#if defined(ICE_64)
	sprintf(s, "%ld", n);
#else
#   ifdef _WIN32
	sprintf(s, "%I64d", n);
#   else
	sprintf(s, "%lld", n);
#   endif
#endif
	Identity id;

	id.name = s;
	ids.clear();
	ids.push_back(id);

	_nameIdentitiesDict.put(NameIdentitiesDict::value_type(string("ID"), ids));

	id.name = s;
	id.category = "contact";
	return id;
    }
    catch(const Freeze::DBException& ex)
    {
	DatabaseException e;
	e.message = ex.message;
	throw e;
    }
}

//
// Called with the RWRecMutex already acquired.
//
void
PhoneBookI::removeI(const Identity& ident, const string& name)
{
    //
    // See the comment in getNewIdentity why the prefix "N" is
    // needed.
    //
    NameIdentitiesDict::iterator p = _nameIdentitiesDict.find("N" + name);

    //
    // If the name isn't found then raise a record not found
    // exception.
    //
    if(p == _nameIdentitiesDict.end())
    {
	throw Freeze::DBNotFoundException(__FILE__, __LINE__);
    }

    Identities identities  = p->second;
    identities.erase(remove_if(identities.begin(), identities.end(), bind2nd(equal_to<Ice::Identity>(), ident)),
		     identities.end());

    if(identities.empty())
    {
	_nameIdentitiesDict.erase(p);
    }
    else
    {
	//
	// See the comment in getNewIdentity why the prefix "N" is
	// needed.
	//
	_nameIdentitiesDict.put(NameIdentitiesDict::value_type("N" + name, identities));
    }
}
