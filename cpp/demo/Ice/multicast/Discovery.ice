// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef DISCOVERY_ICE
#define DISCOVERY_ICE

module Demo
{

interface DiscoverReply
{
    void reply(Object* obj);
};

interface Discover
{
    void lookup(DiscoverReply* replyHere);
};

};

#endif
