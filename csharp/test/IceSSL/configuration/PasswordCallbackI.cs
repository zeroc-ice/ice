//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Security;
using Test;

public class PasswordCallback : IceSSL.IPasswordCallback
{
    public PasswordCallback() => _password = CreateSecureString("password");

    public PasswordCallback(string password) => _password = CreateSecureString(password);

    public SecureString GetPassword(string file) => _password;

    public SecureString GetImportPassword(string file)
    {
        TestHelper.Assert(false);
        return null;
    }

    private static SecureString CreateSecureString(string s)
    {
        SecureString result = new SecureString();
        foreach (char ch in s)
        {
            result.AppendChar(ch);
        }
        return result;
    }

    private SecureString _password;
}
