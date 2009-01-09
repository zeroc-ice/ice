// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

//
// A password callback is an alternate way of supplying the plug-in with
// passwords that avoids using plaintext configuration properties.
//
public interface PasswordCallback
{
    //
    // Obtain the password for the key. If an alias was selected using
    // the IceSSL.Alias property, the value of the property is provided.
    // The return value must not be null.
    //
    char[] getPassword(String alias);

    //
    // Obtain the password for validating the truststore. Return null
    // to skip truststore validation.
    //
    char[] getTruststorePassword();

    //
    // Obtain the password for validating the keystore. Return null
    // to skip keystore validation.
    //
    char[] getKeystorePassword();
}
