//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.Security;

public class PasswordCallback : IceSSL.IPasswordCallback
{
    public PasswordCallback() => _password = createSecureString("password");

    public PasswordCallback(string password) => _password = createSecureString(password);

    public SecureString getPassword(string file) => _password;

    public SecureString getImportPassword(string file)
    {
        Debug.Assert(false);
        return null;
    }

    private static SecureString createSecureString(string s)
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
