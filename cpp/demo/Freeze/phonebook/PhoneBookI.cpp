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
#include <Evictor.h>
#include <sstream>

using namespace std;
using namespace Ice;

EntryI::EntryI(const PhoneBookIPtr& phoneBook, const EvictorPtr& evictor) :
    _phoneBook(phoneBook),
    _evictor(evictor)
{
}

void
EntryI::setIdentity(const string& identity)
{
    _identity = identity;
}

string
EntryI::getName()
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    return _name;
}

void
EntryI::setName(const string& name)
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    assert(!_identity.empty());
    _phoneBook->move(_identity, _name, name);
    _name = name;
}

string
EntryI::getAddress()
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    return _address;
}

void
EntryI::setAddress(const string& address)
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    _address = address;
}

string
EntryI::getPhone()
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    return _phone;
}

void
EntryI::setPhone(const string& phone)
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    _phone = phone;
}

void
EntryI::destroy()
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    assert(!_identity.empty());
    _phoneBook->remove(_identity, _name);
    _evictor->destroyObject(_identity);
}

PhoneBookI::PhoneBookI(const ObjectAdapterPtr& adapter, const EvictorPtr& evictor) :
    _adapter(adapter),
    _evictor(evictor),
    _nextEntryIdentity(0)
{
}

class IdentityToEntry
{
public:

    IdentityToEntry(const ObjectAdapterPtr& adapter) :
	_adapter(adapter)
    {
    }

    EntryPrx operator()(const string& identity)
    {
	return EntryPrx::uncheckedCast(_adapter->createProxy(identity));
    }

private:

    ObjectAdapterPtr _adapter;
};

EntryPrx
PhoneBookI::createEntry()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock
    
    //
    // Get a unique ID
    //
    string identity = "phonebook.entry#";
#ifdef WIN32 // COMPILERBUG
    char s[20];
    sprintf(s, "%I64d", _nextEntryIdentity++);
    identity += s;
#else
    ostringstream s;
    s << _nextEntryIdentity++;
    identity += s.str();
#endif
    
    //
    // Create a new entry Servant.
    //
    EntryIPtr entry = new EntryI(this, _evictor);
    entry->setIdentity(identity);

    //
    // Create a new Ice Object in the evictor, using the new identity
    // and the new Servant.
    //
    _evictor->createObject(identity, entry);

    //
    // Add identity to our name/identity map. The initial name is the
    // empty string.
    //
    _nameIdentitiesDict[""].push_back(identity);

    //
    // Turn the identity into a Proxy and return the Proxy to the
    // caller.
    //
    return IdentityToEntry(_adapter)(identity);
}

Entries
PhoneBookI::findEntries(const string& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    //
    // Lookup all phone book entries that match a name, and return
    // them to the caller.
    //
    NameIdentitiesDict::iterator p = _nameIdentitiesDict.find(name);
    if (p != _nameIdentitiesDict.end())
    {
	Entries entries;
	entries.reserve(p->second.size());
	transform(p->second.begin(), p->second.end(), back_inserter(entries), IdentityToEntry(_adapter));
	return entries;
    }
    else
    {
	return Entries();
    }
}

Names
PhoneBookI::getAllNames()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    //
    // Get all names from this phone book.
    //
    Names names;
    for (NameIdentitiesDict::iterator p = _nameIdentitiesDict.begin(); p != _nameIdentitiesDict.end(); ++p)
    {
	//
	// If there are multiple entries for one name, I want the name
	// listed just as many times.
	//
	for (Identities::size_type i = 0; i < p->second.size(); ++i)
	{
	    names.push_back(p->first);
	}
    }

    return names;
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
    // Called by EntryI to remove itself from the phone book.
    //
    NameIdentitiesDict::iterator p = _nameIdentitiesDict.find(name);
    assert(p != _nameIdentitiesDict.end());
    p->second.erase(remove_if(p->second.begin(), p->second.end(), bind2nd(equal_to<string>(), identity)),
		    p->second.end());
}

void
PhoneBookI::move(const string& identity, const string& oldName, const string& newName)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    //
    // Called by EntryI in case the name has been changed.
    //
    remove(identity, oldName);
    _nameIdentitiesDict[newName].push_back(identity);
}
