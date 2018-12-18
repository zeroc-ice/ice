// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceSSL;

/**
 * A password callback is an alternate way to supply the plug-in with
 * passwords; this avoids using plain text configuration properties.
 **/
public interface PasswordCallback
{
    /**
     * Returns the password for the key. If an alias was selected
     * by setting the <code>IceSSL.Alias</code> property, <code>alias</code>
     * contains the property's value.
     *
     * @param alias The value of the property <code>IceSSL.Alias</code>, if that
     * property is set; <code>null</code>, otherwise.
     * @return The password for the key. The return value must not be <code>null</code>.
     *
     **/
    char[] getPassword(String alias);

    /**
     * Returns the password for validating the truststore.
     *
     * @return The password. To skip truststore validation,
     * return <code>null</code>.
     **/
    char[] getTruststorePassword();

    /**
     * Returns the password for validating the keystore.
     *
     * @return The password. To skip keystore validation,
     * return <code>null</code>.
     **/
    char[] getKeystorePassword();
}
