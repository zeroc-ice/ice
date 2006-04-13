// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
    // Return true to allow a connection using the provided certificate
    // information, or false to reject the connection.
    // 
    boolean verify(VerifyInfo info);
}
