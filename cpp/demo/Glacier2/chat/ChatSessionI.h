// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CHAT_SESSION_I_H
#define CHAT_SESSION_I_H

#include <Chat.h>

class ChatSessionI : public Demo::ChatSession, public IceUtil::Mutex
{
public:

    ChatSessionI(const std::string&);

    virtual void ice_ping(const Ice::Current&) const;
    virtual void setCallback(const Demo::ChatCallbackPrx&, const Ice::Current&);
    virtual void say(const std::string&, const Ice::Current&);
    virtual void destroy(const Ice::Current&);

private:

    const std::string _userId;
    Demo::ChatCallbackPrx _callback;
};

#endif
