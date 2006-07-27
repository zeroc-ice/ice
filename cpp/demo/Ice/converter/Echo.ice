// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ECHO_ICE
#define ECHO_ICE

module Demo
{

interface Echo
{
    ["cpp:const"] idempotent string echoString(string s);
    idempotent void shutdown();
};

};

#endif
