// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

/**
 * An application can customize the certificate verification process
 * by implementing the CertificateVerifier interface.
 **/
public interface CertificateVerifier
{
    /**
     * Determines whether a connection should be accepted or rejected.
     *
     * @param info The details of the connection.
     * @return <code>true</code> if the connection should be accepted;
     * <code>false</code>, otherwise.
     **/
    boolean verify(NativeConnectionInfo info);
}
