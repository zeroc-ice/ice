// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.ejb;

import javax.ejb.Local;
import com.zeroc.ice.Test.Account;
import com.zeroc.ice.Test.AccountNotExistException;

@Local
public interface Service
{
    Account getAccount(String id);

    void addAccount(Account s);
}
