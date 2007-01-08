// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CHAT_ICE
#define CHAT_ICE

#include <Glacier2/Session.ice>

module Demo
{

interface ChatCallback
{
    void message(string data);
};

interface ChatSession extends Glacier2::Session
{
    void setCallback(ChatCallback* callback);
    void say(string data);
};

};

#endif
