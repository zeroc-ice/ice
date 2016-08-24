// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceSSL;

public final class Util
{
    //
    // Create a certificate from a PEM-encoded string.
    //
    public static java.security.cert.X509Certificate
    createCertificate(String certPEM)
        throws java.security.cert.CertificateException
    {
        final String header = "-----BEGIN CERTIFICATE-----";
        final String footer = "-----END CERTIFICATE-----";

        //
        // The generateCertificate method requires that its input begin
        // with the PEM header.
        //
        int pos = certPEM.indexOf(header);
        if(pos == -1)
        {
            certPEM = header + "\n" + certPEM;
        }
        else if(pos > 0)
        {
            certPEM = certPEM.substring(pos);
        }

        //
        // Add the footer if necessary.
        //
        if(certPEM.indexOf(footer) == -1)
        {
            certPEM = certPEM + footer;
        }

        byte[] bytes = null;
        try
        {
            bytes = certPEM.getBytes("UTF8");
        }
        catch(java.io.UnsupportedEncodingException ex)
        {
            assert(false);
            return null;
        }

        java.io.ByteArrayInputStream in = new java.io.ByteArrayInputStream(bytes);
        java.security.cert.CertificateFactory cf = java.security.cert.CertificateFactory.getInstance("X.509");
        return (java.security.cert.X509Certificate)cf.generateCertificate(in);
    }

    public final static String jdkTarget = "1.5";

    //
    // Needed by the test scripts to determine the JDK target of the SSL plug-in.
    //
    public static void
    main(String[] args)
    {
        System.out.println(jdkTarget);
    }
}
