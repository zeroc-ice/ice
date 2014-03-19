// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module Demo
{

["java:serializable:Demo.MyGreeting"] sequence<byte> Greeting;

interface Greet
{
    idempotent void sendGreeting(Greeting g);
    void shutdown();
};

};
