// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "common.ice"

[["java:package:com.zeroc.ice"]]
module Test
{

class Account extends Base
{
    string id;
};

interface Service
{
    void setAccount(Account s);

    Account getAccount();
};

};
