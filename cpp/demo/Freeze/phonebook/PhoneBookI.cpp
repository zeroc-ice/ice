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
    _phoneBook->remove(_identity, _name);
    _evictor->destroyObject(_identity);
}

PhoneBookI::PhoneBookI(const ObjectAdapterPtr& adapter, const EvictorPtr& evictor) :
    _adapter(adapter),
    _evictor(evictor),
    _nextEntryIdentity(0)
{
}

EntryPrx
PhoneBookI::createEntry()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    
#ifdef WIN32 // COMPILERBUG
    char s[20];
    sprintf(s, "%I64d", _nextEntryIdentity++);
    string identity = s;
#else
    ostringstream s;
    s << _nextEntryIdentity++;
    string identity = s.str();
#endif
    
    EntryPtr entry = new EntryI(this, _evictor);
    _evictor->createObject(identity, entry);

    add(identity, "");

    return EntryPrx::uncheckedCast(_adapter->createProxy(identity));
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

Entries
PhoneBookI::findEntries(const string& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

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
PhoneBookI::add(const string& identity, const string& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    _nameIdentitiesDict[name].push_back(identity);
}

void
PhoneBookI::remove(const string& identity, const string& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    NameIdentitiesDict::iterator p = _nameIdentitiesDict.find(name);
    assert(p != _nameIdentitiesDict.end());
    p->second.erase(remove_if(p->second.begin(), p->second.end(), bind2nd(equal_to<string>(), identity)),
		    p->second.end());
}

void
PhoneBookI::move(const string& identity, const string& oldName, const string& newName)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    remove(identity, oldName);
    add(identity, newName);
}
