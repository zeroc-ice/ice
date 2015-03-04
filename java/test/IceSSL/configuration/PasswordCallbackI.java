// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceSSL.configuration;
import test.IceSSL.configuration.Test.*;

public class PasswordCallbackI implements IceSSL.PasswordCallback
{
    public
    PasswordCallbackI()
    {
        _password = "password";
    }

    public
    PasswordCallbackI(String password)
    {
        _password = password;
    }

    public char[]
    getPassword(String alias)
    {
        return _password.toCharArray();
    }

    public char[]
    getTruststorePassword()
    {
        return null;
    }

    public char[]
    getKeystorePassword()
    {
        return null;
    }

    private String _password;
}
