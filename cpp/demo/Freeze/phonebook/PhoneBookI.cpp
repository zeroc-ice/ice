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

EntryI::EntryI(const string& identity, const PhoneBookIPtr& phoneBook) :
    _identity(identity),
    _phoneBook(phoneBook)
{
}

EntryI::~EntryI()
{
}

wstring
EntryI::getName()
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    return _name;
}

void
EntryI::setName(const wstring& name)
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    _phoneBook->move(_identity, _name, name);
    _name = name;
}

wstring
EntryI::getAddress()
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    return _address;
}

void
EntryI::setAddress(const wstring& address)
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    _address = address;
}

string
EntryI::getNumber()
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    return _number;
}

void
EntryI::setNumber(const string& number)
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    _number = number;
}

void
EntryI::destroy()
{
    JTCSyncT<JTCMutex> sync(*this); // TODO: Reader/Writer lock
    _phoneBook->remove(_identity, _name);
}

PhoneBookI::PhoneBookI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

PhoneBookI::~PhoneBookI()
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
    
    add(identity, L"");

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
PhoneBookI::findEntries(const wstring& name)
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
PhoneBookI::listNames()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    Names names;
    for (NameIdentitiesDict::iterator p = _nameIdentitiesDict.begin(); p != _nameIdentitiesDict.begin(); ++p)
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
PhoneBookI::add(const string& identity, const wstring& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    _nameIdentitiesDict[name].push_back(identity);
}

void
PhoneBookI::remove(const string& identity, const wstring& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    NameIdentitiesDict::iterator p = _nameIdentitiesDict.find(name);
    assert(p != _nameIdentitiesDict.end());
    p->second.erase(remove_if(p->second.begin(), p->second.end(), bind2nd(equal_to<string>(), identity)),
		    p->second.end());
}

void
PhoneBookI::move(const string& identity, const wstring& oldName, const wstring& newName)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this); // TODO: Reader/Writer lock

    remove(identity, oldName);
    add(identity, newName);
}
