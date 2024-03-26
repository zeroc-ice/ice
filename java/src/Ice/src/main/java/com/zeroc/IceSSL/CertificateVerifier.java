//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceSSL;

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
    boolean verify(ConnectionInfo info);
}
