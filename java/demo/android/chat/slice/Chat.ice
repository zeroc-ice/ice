// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

/**
 *
 * The Chat module defines types shared by definitions from
 * ChatSession.ice and PollingChat.ice.
 *
 **/
module Chat
{

/**
 *
 * The InvalidMessageException is raised when a user sends an invalid
 * message to the server. A message is considered invalid if the
 * message size exceeds the maximum message size.
 *
 **/
exception InvalidMessageException
{
    /**
     *
     * The reason why the message was rejected by the server.
     *
     **/
    string reason;
};

};
