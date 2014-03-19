// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module Test
{

interface Server
{
    void waitTestSuccess();
    void terminate();
};

interface Controller
{
    Server* runServer(string lang, string name, string protocol, string host);
};

};
