// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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

    static IceUtil::Mutex* _instanceMutex;
private:
    
    list<Demo::ChatCallbackPrx> _members;

    static ChatRoomPtr _instance;
};

ChatRoomPtr ChatRoom::_instance;
IceUtil::Mutex* ChatRoom::_instanceMutex = 0;

namespace
{


class Init
{
public:

    Init()
    {
        ChatRoom::_instanceMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete ChatRoom::_instanceMutex;
        ChatRoom::_instanceMutex = 0;
    }
};

Init init;

}

class AMI_ChatCallback_messageI : public Demo::AMI_ChatCallback_message
{
public:

    virtual void ice_response()
    {
    }

    virtual void ice_exception(const Ice::Exception&)
    {
    }
};

ChatRoomPtr&
ChatRoom::instance()
{
    IceUtil::Mutex::Lock sync(*_instanceMutex);
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
    _members.push_back(callback);
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
        (*p)->message_async(new AMI_ChatCallback_messageI(), data);
    }
}

ChatSessionI::ChatSessionI(const string& userId) :
    _userId(userId)
{
}

void
ChatSessionI::setCallback(const ChatCallbackPrx& callback, const Ice::Current&)
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
