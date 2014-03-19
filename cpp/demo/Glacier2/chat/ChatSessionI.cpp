// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

    ChatRoom();
    ~ChatRoom();

    static ChatRoomPtr& instance();

    void enter(const Demo::ChatSessionPrx&, const Demo::ChatCallbackPrx&);
    void leave(const Demo::ChatCallbackPrx&);
    void message(const string&) const;
    void update(const Demo::ChatCallbackPrx&);

    struct MemberInfo
    {
        Demo::ChatSessionPrx session;
        Demo::ChatCallbackPrx callback;
        IceUtil::Time updateTime;
    };
    list<MemberInfo> members() const;

    static IceUtil::Mutex* _instanceMutex;
private:
    
    list<MemberInfo> _members;
    IceUtil::TimerPtr _timer;

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

class ReapTask : public IceUtil::TimerTask
{
public:

    virtual void runTimerTask()
    {
        ChatRoomPtr chatRoom = ChatRoom::instance();
        list<ChatRoom::MemberInfo> members = chatRoom->members();
        IceUtil::Time now = IceUtil::Time::now();
        for(list<ChatRoom::MemberInfo>::const_iterator p = members.begin(); p != members.end(); ++p)
        {
            if(now - p->updateTime > IceUtil::Time::secondsDouble(30 * 1.5)) // SessionTimeout * 1.5
            {
                try
                {
                    p->session->destroy();
                }
                catch(const Ice::Exception&)
                {
                    // Ignore
                }
            }
        }
    }
};

}

ChatRoom::ChatRoom()
{
    _timer = new IceUtil::Timer();
    _timer->scheduleRepeated(new ReapTask(), IceUtil::Time::seconds(30));
}

ChatRoom::~ChatRoom()
{
    _timer->destroy();
}

ChatRoomPtr&
ChatRoom::instance()
{
    IceUtil::Mutex::Lock sync(*_instanceMutex);
    if(!_instance)
    {
        _instance = new ChatRoom();
    }

    return _instance;
}

void
ChatRoom::enter(const Demo::ChatSessionPrx& session, const ChatCallbackPrx& callback)
{
    Lock sync(*this);
    MemberInfo info;
    info.session = session;
    info.callback = callback;
    info.updateTime = IceUtil::Time::now();
    _members.push_back(info);
}

void
ChatRoom::leave(const ChatCallbackPrx& callback)
{
    Lock sync(*this);
    list<MemberInfo>::iterator p;
    for(p = _members.begin(); p != _members.end(); ++p)
    {
        if(Ice::proxyIdentityEqual(callback, p->callback))
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
    for(list<MemberInfo>::const_iterator p = _members.begin(); p != _members.end(); ++p)
    {
        p->callback->begin_message(data);
    }
}

void
ChatRoom::update(const ChatCallbackPrx& callback)
{
    Lock sync(*this);
    for(list<MemberInfo>::iterator p = _members.begin(); p != _members.end(); ++p)
    {
        if(Ice::proxyIdentityEqual(callback, p->callback))
        {
            p->updateTime = IceUtil::Time::now();
            break;
        }
    }
}

list<ChatRoom::MemberInfo>
ChatRoom::members() const
{
    return _members;
}

ChatSessionI::ChatSessionI(const string& userId) :
    _userId(userId)
{
}

void
ChatSessionI::ice_ping(const Ice::Current&) const
{
    Lock sync(*this);
    ChatRoom::instance()->update(_callback);
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
        chatRoom->enter(ChatSessionPrx::uncheckedCast(current.adapter->createProxy(current.id)), callback);
    }
}

void
ChatSessionI::say(const string& data, const Ice::Current&)
{
    Lock sync(*this);
    ChatRoomPtr chatRoom = ChatRoom::instance();
    chatRoom->message(_userId + " says: " + data);
    chatRoom->update(_callback);
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
