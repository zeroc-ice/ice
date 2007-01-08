// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

//
// An application can customize the certificate verification process
// by implementing the CertificateVerifier interface.
//
public interface CertificateVerifier
{
    //
    // Return false if the connection should be rejected, or true to
    // allow it.
    // 
    boolean verify(ConnectionInfo info);
}
