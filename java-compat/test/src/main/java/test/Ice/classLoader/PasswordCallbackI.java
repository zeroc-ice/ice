// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.classLoader;

public class PasswordCallbackI implements IceSSL.PasswordCallback
{
    @Override
    public char[] getPassword(String alias)
    {
        return "password".toCharArray();
    }

    @Override
    public char[] getTruststorePassword()
    {
        return "password".toCharArray();
    }

    @Override
    public char[] getKeystorePassword()
    {
        return "password".toCharArray();
    }
}
