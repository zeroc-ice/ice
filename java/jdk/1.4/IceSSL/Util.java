// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

public final class Util
{
    public static ConnectionInfo
    getConnectionInfo(Ice.Connection connection)
    {
        Ice.ConnectionI con = (Ice.ConnectionI)connection;
        assert(con != null);

        //
        // Lock the connection directly. This is done because the only
        // thing that prevents the transceiver from being closed during
        // the duration of the invocation is the connection.
        //
        synchronized(con)
        {
            IceInternal.Transceiver transceiver = con.getTransceiver();
            if(transceiver == null)
            {
                ConnectionInvalidException ex = new ConnectionInvalidException();
                ex.reason = "connection closed";
                throw ex;
            }

            try
            {
                TransceiverI sslTransceiver = (TransceiverI)transceiver;
                return sslTransceiver.getConnectionInfo();
            }
            catch(ClassCastException ex)
            {
                ConnectionInvalidException e = new ConnectionInvalidException();
                e.reason = "not ssl connection";
                throw e;
            }
        }
    }

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

    static ConnectionInfo
    populateConnectionInfo(javax.net.ssl.SSLSocket fd, String adapterName, boolean incoming)
    {
        ConnectionInfo info = new ConnectionInfo();
        javax.net.ssl.SSLSession session = fd.getSession();
        try
        {
            info.certs = session.getPeerCertificates();
        }
        catch(javax.net.ssl.SSLPeerUnverifiedException ex)
        {
            // No peer certificates.
        }
        info.cipher = session.getCipherSuite();
        info.localAddr = (java.net.InetSocketAddress)fd.getLocalSocketAddress();
        info.remoteAddr = (java.net.InetSocketAddress)fd.getRemoteSocketAddress();
        info.adapterName = adapterName;
        info.incoming = incoming;
        return info;
    }

    public final static String jdkTarget = "1.4";

    //
    // Needed by the test scripts to determine the JDK target of the SSL plug-in.
    //
    public static void
    main(String[] args)
    {
        System.out.println(jdkTarget);
    }
}
