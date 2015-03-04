// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

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
