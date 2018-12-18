// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.IceSSL.configuration;

public class CertificateVerifierI implements IceSSL.CertificateVerifier
{
    public
    CertificateVerifierI()
    {
        reset();
    }

    @Override
    public boolean
    verify(IceSSL.ConnectionInfo info)
    {
        if(info.certs != null)
        {
            try
            {
                java.util.Collection<java.util.List<?> > subjectAltNames =
                    ((java.security.cert.X509Certificate)info.certs[0]).getSubjectAlternativeNames();
                test(subjectAltNames != null);
                java.util.List<String> ipAddresses = new java.util.ArrayList<String>();
                java.util.List<String> dnsNames = new java.util.ArrayList<String>();
                for(java.util.List<?> l : subjectAltNames)
                {
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
