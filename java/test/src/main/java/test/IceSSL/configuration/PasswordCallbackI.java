//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.IceSSL.configuration;

public class PasswordCallbackI implements com.zeroc.IceSSL.PasswordCallback
{
    public PasswordCallbackI()
    {
        _password = "password";
    }

    public PasswordCallbackI(String password)
    {
        _password = password;
    }

    @Override
    public char[] getPassword(String alias)
    {
        return _password.toCharArray();
    }

    @Override
    public char[] getTruststorePassword()
    {
        return null;
    }

    @Override
    public char[] getKeystorePassword()
    {
        return null;
    }

    private String _password;
}
