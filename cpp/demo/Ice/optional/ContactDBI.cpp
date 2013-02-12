// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <ContactDBI.h>

using namespace std;
using namespace Demo;

void
ContactDBI::addContact(const std::string& name, const IceUtil::Optional< NumberType>& type,
    const IceUtil::Optional< std::string>& number, const IceUtil::Optional< Ice::Int>& dialGroup,
    const Ice::Current&)
{
    ContactPtr contact = new Contact();
    contact->name = name;
    if(type)
    {
    	contact->type = type;
    }
    if(number)
    {
    	contact->number = number;
    }
    if(dialGroup)
    {
    	contact->dialGroup = dialGroup;
    }
    pair<map<string, ContactPtr>::iterator, bool> p = _contacts.insert(make_pair(name, contact));
    if(!p.second)
    {
	p.first->second = contact;
    }
}

void
ContactDBI::updateContact(const std::string& name, const IceUtil::Optional< NumberType>& type,
    const IceUtil::Optional< std::string>& number, const IceUtil::Optional< Ice::Int>& dialGroup,
    const Ice::Current&)
{
    map<string, ContactPtr>::iterator p = _contacts.find(name);
    if(p != _contacts.end())
    {
	if(type)
	{
	    p->second->type = type;
	}
	if(number)
	{
	    p->second->number = number;
	}
	if(dialGroup)
	{
	    p->second->dialGroup = dialGroup;
	}
    }
}

ContactPtr
ContactDBI::query(const std::string& name, const Ice::Current&)
{
    map<string, ContactPtr>::const_iterator p = _contacts.find(name);
    if(p != _contacts.end())
    {
    	return p->second;
    }
    return 0;
}

IceUtil::Optional<std::string>
ContactDBI::queryNumber(const std::string& name, const Ice::Current&)
{
    map<string, ContactPtr>::const_iterator p = _contacts.find(name);
    if(p != _contacts.end())
    {
    	return p->second->number;
    }
    return IceUtil::None;
}

void
ContactDBI::queryDialgroup(const std::string& name, IceUtil::Optional< Ice::Int>& dialGroup, const Ice::Current&)
{
    map<string, ContactPtr>::const_iterator p = _contacts.find(name);
    if(p != _contacts.end())
    {
    	dialGroup = p->second->dialGroup;
    }
}

void
ContactDBI::shutdown(const Ice::Current& c)
{
    cout << "Shutting down..." << endl;
    c.adapter->getCommunicator()->shutdown();
}
