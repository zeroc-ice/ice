//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
