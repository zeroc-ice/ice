// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class CertificateVerifierI implements IceSSL.CertificateVerifier
{
    public
    CertificateVerifierI()
    {
        reset();
    }

    public boolean
    verify(IceSSL.ConnectionInfo info)
    {
        if(info.certs != null)
        {
            try
            {
                java.util.Collection subjectAltNames =
                    ((java.security.cert.X509Certificate)info.certs[0]).getSubjectAlternativeNames();
                test(subjectAltNames != null);
                java.util.ArrayList ipAddresses = new java.util.ArrayList();
                java.util.ArrayList dnsNames = new java.util.ArrayList();
                java.util.Iterator i = subjectAltNames.iterator();
                while(i.hasNext())
                {
                    java.util.List l = (java.util.List)i.next();
                    test(!l.isEmpty());
                    Integer n = (Integer)l.get(0);
                    if(n.intValue() == 7)
                    {
                        ipAddresses.add((String)l.get(1));
                    }
                    else if(n.intValue() == 2)
                    {
                        dnsNames.add((String)l.get(1));
                    }
                }

                test(dnsNames.contains("server"));
                test(ipAddresses.contains("127.0.0.1"));
            }
            catch(java.security.cert.CertificateParsingException ex)
            {
                test(false);
            }
        }

        _hadCert = info.certs != null;
        _invoked = true;
        return _returnValue;
    }

    void
    reset()
    {
        _returnValue = true;
        _invoked = false;
        _hadCert = false;
    }

    void
    returnValue(boolean b)
    {
        _returnValue = b;
    }

    boolean
    invoked()
    {
        return _invoked;
    }

    boolean
    hadCert()
    {
        return _hadCert;
    }

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private boolean _returnValue;
    private boolean _invoked;
    private boolean _hadCert;
}
