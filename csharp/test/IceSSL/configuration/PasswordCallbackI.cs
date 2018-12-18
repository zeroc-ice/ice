// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Security;

public class PasswordCallbackI : IceSSL.PasswordCallback
{
    public PasswordCallbackI()
    {
        _password = createSecureString("password");
    }

    public PasswordCallbackI(string password)
    {
        _password = createSecureString(password);
    }

    public SecureString getPassword(string file)
    {
        return _password;
    }

    public SecureString getImportPassword(string file)
    {
        Debug.Assert(false);
        return null;
    }

    private static SecureString createSecureString(string s)
    {
        SecureString result = new SecureString();
        foreach(char ch in s)
        {
            result.AppendChar(ch);
        }
        return result;
    }

    private SecureString _password;
}
