// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <ChatSessionI.h>

using namespace std;
using namespace Demo;

ChatRoomMembers::ChatRoomMembers()
{
}

void
ChatRoomMembers::add(const ChatCallbackPrx& callback)
{
    IceUtil::Mutex::Lock sync(*this);
    _members.push_back(callback);
}

void
ChatRoomMembers::remove(const ChatCallbackPrx& callback)
{
    IceUtil::Mutex::Lock sync(*this);
    list<ChatCallbackPrx>::iterator p = find(_members.begin(), _members.end(), callback);
    if(p != _members.end())
    {
	_members.erase(p);
    }
}

void
ChatRoomMembers::message(const string& data)
{
    IceUtil::Mutex::Lock sync(*this);
    list<ChatCallbackPrx>::iterator p = _members.begin();
    while(p != _members.end())
    {
	try
	{
	    (*p)->message(data);
	}
	catch(const Ice::LocalException&)
	{
	    p = _members.erase(p);
	    continue;
	}
	++p;
    }
}

ChatSessionI::ChatSessionI(const ChatRoomMembersPtr& members, const string& userId) :
    _members(members),
    _userId(userId),
    _destroy(false)
{
    _members->message(_userId + " has entered the chat room.");
}

void
ChatSessionI::setCallback(const ChatCallbackPrx& callback, const Ice::Current& current)
{
    IceUtil::Mutex::Lock sync(*this);
    if(!_callback)
    {
	_callback = callback;
	_members->add(callback);
    }
}

void
ChatSessionI::say(const string& data, const Ice::Current&)
{
    _members->message(_userId + " says: " + data);
}

void
ChatSessionI::destroy(const Ice::Current& current)
{
    IceUtil::Mutex::Lock sync(*this);
    if(!_destroy)
    {
	_destroy = true;
	if(_callback)
	{
	    _members->remove(_callback);
	    _callback = 0;
	}
	current.adapter->remove(current.id);
	_members->message(_userId + " has left the chat room.");
    }
}

