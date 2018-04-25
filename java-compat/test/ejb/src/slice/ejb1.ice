// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "common.ice"

[["java:package:com.zeroc.ice"]]
module Test
{

exception AccountNotExistException
{
}

class Account extends Base
{
    string id;
}

interface Database
{
    void addAccount(Account s);

    Account getAccount(string id)
        throws AccountNotExistException;
}

interface Service
{
    void addAccount(Account s);

    Account getAccount(string id);
}

}
