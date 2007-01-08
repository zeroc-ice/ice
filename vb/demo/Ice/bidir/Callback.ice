// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CALLBACK_ICE
#define CALLBACK_ICE

#include <Ice/Identity.ice>

module BidirDemo
{

interface CallbackReceiver
{
    void callback(int num);
};

interface CallbackSender
{
    void addClient(Ice::Identity ident);
};

};

#endif
