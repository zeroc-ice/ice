// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CHAT_SESSION_I_H
#define CHAT_SESSION_I_H

#include <Chat.h>

#include <list>

class ChatRoomMembers : public IceUtil::Mutex, public IceUtil::Shared
{
public:

    ChatRoomMembers();

    void add(const ::Demo::ChatCallbackPrx&);
    void remove(const ::Demo::ChatCallbackPrx&);
    void message(const std::string&);

private:
    
    std::list<::Demo::ChatCallbackPrx> _members;
};
typedef IceUtil::Handle<ChatRoomMembers> ChatRoomMembersPtr;

class ChatSessionI : public ::Demo::ChatSession, public IceUtil::Mutex
{
public:

    ChatSessionI(const ChatRoomMembersPtr&, const std::string&);

    virtual void setCallback(const ::Demo::ChatCallbackPrx&, const Ice::Current&);
    virtual void say(const std::string&, const Ice::Current&);
    virtual void destroy(const Ice::Current&);

private:

    ChatRoomMembersPtr _members;
    ::std::string _userId;
    ::Demo::ChatCallbackPrx _callback;
    bool _destroy;
};

#endif
