// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.classLoader;

public class PasswordCallbackI implements IceSSL.PasswordCallback
{
    public char[] getPassword(String alias)
    {
        return "password".toCharArray();
    }

    public char[] getTruststorePassword()
    {
        return "password".toCharArray();
    }

    public char[] getKeystorePassword()
    {
        return "password".toCharArray();
    }
}
