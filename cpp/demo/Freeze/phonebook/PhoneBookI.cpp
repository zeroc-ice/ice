// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <PhoneBookI.h>
#include <sstream>

using namespace std;
using namespace Ice;

EntryI::EntryI(const string& identity, const ObjectAdapterPtr& adapter, const PhoneBookIPtr& phoneBook) :
    _identity(identity),
    _adapter(adapter),
    _phoneBook(phoneBook)
{
    __setNoDelete(true);

    try
    {
	EntryPrx self = EntryPrx::uncheckedCast(_adapter->add(this, _identity));
    }
    catch(...)
    {
	__setNoDelete(false);
	throw;
    }

    __setNoDelete(false);
}

EntryI::~EntryI()
{
}

wstring
EntryI::getName()
{
    JTCSyncT<JTCMutex> sync(*this);
    return _name;
}

void
EntryI::setName(const wstring& name)
{
    JTCSyncT<JTCMutex> sync(*this);
    _phoneBook->move(_identity, _name, name);
    _name = name;
}

wstring
EntryI::getAddress()
{
    JTCSyncT<JTCMutex> sync(*this);
    return _address;
}

void
EntryI::setAddress(const wstring& address)
{
    JTCSyncT<JTCMutex> sync(*this);
    _address = address;
}

string
EntryI::getNumber()
{
    JTCSyncT<JTCMutex> sync(*this);
    return _number;
}

void
EntryI::setNumber(string number)
{
    JTCSyncT<JTCMutex> sync(*this);
    _number = number;
}

void
EntryI::destroy()
{
    JTCSyncT<JTCMutex> sync(*this);
    _adapter->remove(_identity);
    _phoneBook->remove(_identity, _name);
}

PhoneBookI::PhoneBookI(const ObjectAdapterPtr& adapter) :
    _adapter(adapter),
    _nextEntryIdentity(0)
{
}

PhoneBookI::~PhoneBookI()
{
}

EntryPrx
PhoneBookI::createEntry()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    
    ostringstream s;
    s << _nextEntryIdentity++;
    string identity = s.str();
    
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
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    EntryIdentitiesDict::iterator p = _entryIdentitiesDict.find(name);
    if (p != _entryIdentitiesDict.end())
    {
	Entries entries;
	transform(p->second.begin(), p->second.end(), back_inserter(entries), IdentityToEntry(_adapter));
	return entries;
    }
    else
    {
	return Entries();
    }
}

void
PhoneBookI::add(const string& identity, const wstring& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    _entryIdentitiesDict[name].push_back(identity);
}

void
PhoneBookI::remove(const string& identity, const wstring& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    EntryIdentitiesDict::iterator p = _entryIdentitiesDict.find(name);
    assert(p != _entryIdentitiesDict.end());
    p->second.erase(remove_if(p->second.begin(), p->second.end(), bind2nd(equal_to<string>(), identity)),
		    p->second.end());
}

void
PhoneBookI::move(const string& identity, const wstring& oldName, const wstring& newName)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    remove(identity, oldName);
    add(identity, newName);
}
