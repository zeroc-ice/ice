// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <ChatSessionI.h>
#include <list>

using namespace std;
using namespace Demo;

class ChatRoom;
typedef IceUtil::Handle<ChatRoom> ChatRoomPtr;

class ChatRoom : public IceUtil::Mutex, public IceUtil::Shared
{
public:

    static ChatRoomPtr& instance();

    void enter(const Demo::ChatCallbackPrx&);
    void leave(const Demo::ChatCallbackPrx&);
    void message(const string&) const;

private:
    
    list<Demo::ChatCallbackPrx> _members;

    static ChatRoomPtr _instance;
    static IceUtil::StaticMutex _instanceMutex;
};

ChatRoomPtr ChatRoom::_instance;
IceUtil::StaticMutex ChatRoom::_instanceMutex = ICE_STATIC_MUTEX_INITIALIZER;

ChatRoomPtr&
ChatRoom::instance()
{
    IceUtil::StaticMutex::Lock sync(_instanceMutex);
    if(!_instance)
    {
        _instance = new ChatRoom;
    }

    return _instance;
}

void
ChatRoom::enter(const ChatCallbackPrx& callback)
{
    Lock sync(*this);
    _members.push_back(ChatCallbackPrx::uncheckedCast(callback->ice_oneway()));
}

void
ChatRoom::leave(const ChatCallbackPrx& callback)
{
    Lock sync(*this);
    list<ChatCallbackPrx>::iterator p;
    for(p = _members.begin(); p != _members.end(); ++p)
    {
        if(Ice::proxyIdentityEqual(callback, *p))
        {
            break;
        }
    }

    assert(p != _members.end());
    _members.erase(p);
}

void
ChatRoom::message(const string& data) const
{
    Lock sync(*this);
    for(list<ChatCallbackPrx>::const_iterator p = _members.begin(); p != _members.end(); ++p)
    {
        try
        {
            (*p)->message(data);
        }
        catch(const Ice::LocalException&)
        {
        }
    }
}

ChatSessionI::ChatSessionI(const string& userId) :
    _userId(userId)
{
}

void
ChatSessionI::setCallback(const ChatCallbackPrx& callback, const Ice::Current& current)
{
    Lock sync(*this);
    if(!_callback)
    {
        _callback = callback;
        ChatRoomPtr chatRoom = ChatRoom::instance();
        chatRoom->message(_userId + " has entered the chat room.");
        chatRoom->enter(callback);
    }
}

void
ChatSessionI::say(const string& data, const Ice::Current&)
{
    ChatRoom::instance()->message(_userId + " says: " + data);
}

void
ChatSessionI::destroy(const Ice::Current& current)
{
    Lock sync(*this);
    if(_callback)
    {
        ChatRoomPtr chatRoom = ChatRoom::instance();
        chatRoom->leave(_callback);
        _callback = 0;
        chatRoom->message(_userId + " has left the chat room.");
    }
    current.adapter->remove(current.id);
}
