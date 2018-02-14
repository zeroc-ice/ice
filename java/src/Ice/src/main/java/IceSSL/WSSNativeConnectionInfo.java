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
 *
 * This class is a native extension of the Slice local class
 * IceSSL::WSSConnectionInfo. It provides access to the native
 * Java certificates.
 *
 **/
public class WSSNativeConnectionInfo extends WSSConnectionInfo
{
    /**
     * The certificate chain. This may be null if the peer did not
     * supply a certificate. The peer's certificate (if any) is the
     * first one in the chain.
     **/
    public java.security.cert.Certificate[] nativeCerts;
}
