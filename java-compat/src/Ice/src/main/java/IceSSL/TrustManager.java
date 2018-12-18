// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceSSL;

class TrustManager
{
    TrustManager(Ice.Communicator communicator)
    {
        assert communicator != null;
        _communicator = communicator;
        Ice.Properties properties = communicator.getProperties();
        _traceLevel = properties.getPropertyAsInt("IceSSL.Trace.Security");
        String key = null;
        try
        {
            key = "IceSSL.TrustOnly";
            parse(properties.getProperty(key), _rejectAll, _acceptAll);
            key = "IceSSL.TrustOnly.Client";
            parse(properties.getProperty(key), _rejectClient, _acceptClient);
            key = "IceSSL.TrustOnly.Server";
            parse(properties.getProperty(key), _rejectAllServer, _acceptAllServer);
            java.util.Map<String, String> dict = properties.getPropertiesForPrefix("IceSSL.TrustOnly.Server.");
            for(java.util.Map.Entry<String, String> p : dict.entrySet())
            {
                key = p.getKey();
                String name = key.substring("IceSSL.TrustOnly.Server.".length());
                java.util.List<java.util.List<RFC2253.RDNPair> > reject =
                    new java.util.LinkedList<java.util.List<RFC2253.RDNPair> >();
                java.util.List<java.util.List<RFC2253.RDNPair> > accept =
                    new java.util.LinkedList<java.util.List<RFC2253.RDNPair> >();
                parse(p.getValue(), reject, accept);
                if(!reject.isEmpty())
                {
                    _rejectServer.put(name, reject);
                }
                if(!accept.isEmpty())
                {
                    _acceptServer.put(name, accept);
                }
            }
        }
        catch(RFC2253.ParseException e)
        {
            Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
            ex.reason = "IceSSL: invalid property " + key  + ":\n" + e.reason;
            throw ex;
        }
    }

    boolean
    verify(ConnectionInfo info, String desc)
    {
        java.util.List<java.util.List<java.util.List<RFC2253.RDNPair> > >
            reject = new java.util.LinkedList<java.util.List<java.util.List<RFC2253.RDNPair> > >(),
            accept = new java.util.LinkedList<java.util.List<java.util.List<RFC2253.RDNPair> > >();

        if(!_rejectAll.isEmpty())
        {
            reject.add(_rejectAll);
        }
        if(info.incoming)
        {
            if(!_rejectAllServer.isEmpty())
            {
                reject.add(_rejectAllServer);
            }
            if(info.adapterName.length() > 0)
            {
                java.util.List<java.util.List<RFC2253.RDNPair> > p = _rejectServer.get(info.adapterName);
                if(p != null)
                {
                    reject.add(p);
                }
            }
        }
        else
        {
            if(!_rejectClient.isEmpty())
            {
                reject.add(_rejectClient);
            }
        }

        if(!_acceptAll.isEmpty())
        {
            accept.add(_acceptAll);
        }
        if(info.incoming)
        {
            if(!_acceptAllServer.isEmpty())
            {
                accept.add(_acceptAllServer);
            }
            if(info.adapterName.length() > 0)
            {
                java.util.List<java.util.List<RFC2253.RDNPair> > p = _acceptServer.get(info.adapterName);
                if(p != null)
                {
                    accept.add(p);
                }
            }
        }
        else
        {
            if(!_acceptClient.isEmpty())
            {
                accept.add(_acceptClient);
            }
        }

        //
        // If there is nothing to match against, then we accept the cert.
        //
        if(reject.isEmpty() && accept.isEmpty())
        {
            return true;
        }

        //
        // If there is no certificate then we match false.
        //
        if(info.certs != null && info.certs.length > 0)
        {
            javax.security.auth.x500.X500Principal subjectDN = ((java.security.cert.X509Certificate)info.certs[0]).getSubjectX500Principal();
            String subjectName = subjectDN.getName(javax.security.auth.x500.X500Principal.RFC2253);
            assert subjectName != null;
            try
            {
                //
                // Decompose the subject DN into the RDNs.
                //
                if(_traceLevel > 0)
                {
                    if(info.incoming)
                    {
                        _communicator.getLogger().trace("Security", "trust manager evaluating client:\n" +
                            "subject = " + subjectName + "\n" +
                            "adapter = " + info.adapterName + "\n" +
                            desc);
                    }
                    else
                    {
                        _communicator.getLogger().trace("Security", "trust manager evaluating server:\n" +
                            "subject = " + subjectName + "\n" + desc);
                    }
                }
                java.util.List<RFC2253.RDNPair> dn = RFC2253.parseStrict(subjectName);

                //
                // Fail if we match anything in the reject set.
                //
                for(java.util.List<java.util.List<RFC2253.RDNPair>> matchSet : reject)
                {
                    if(_traceLevel > 1)
                    {
                        StringBuilder s = new StringBuilder("trust manager rejecting PDNs:\n");
                        stringify(matchSet, s);
                        _communicator.getLogger().trace("Security", s.toString());
                    }
                    if(match(matchSet, dn))
                    {
                        return false;
                    }
                }

                //
                // Succeed if we match anything in the accept set.
                //
                for(java.util.List<java.util.List<RFC2253.RDNPair>> matchSet : accept)
                {
                    if(_traceLevel > 1)
                    {
                        StringBuilder s = new StringBuilder("trust manager accepting PDNs:\n");
                        stringify(matchSet, s);
                        _communicator.getLogger().trace("Security", s.toString());
                    }
                    if(match(matchSet, dn))
                    {
                        return true;
                    }
                }
            }
            catch(RFC2253.ParseException e)
            {
                _communicator.getLogger().warning(
                    "IceSSL: unable to parse certificate DN `" + subjectName + "'\nreason: " + e.reason);
            }

            //
            // At this point we accept the connection if there are no explicit accept rules.
            //
            return accept.isEmpty();
        }

        return false;
    }

    private boolean
    match(java.util.List<java.util.List<RFC2253.RDNPair> > matchSet, java.util.List<RFC2253.RDNPair> subject)
    {
        for(java.util.List<RFC2253.RDNPair> r : matchSet)
        {
            if(matchRDNs(r, subject))
            {
                return true;
            }
        }
        return false;
    }

    private boolean
    matchRDNs(java.util.List<RFC2253.RDNPair> match, java.util.List<RFC2253.RDNPair> subject)
    {
        for(RFC2253.RDNPair matchRDN : match)
        {
            boolean found = false;
            for(RFC2253.RDNPair subjectRDN : subject)
            {
                if(matchRDN.key.equals(subjectRDN.key))
                {
                    found = true;
                    if(!matchRDN.value.equals(subjectRDN.value))
                    {
                        return false;
                    }
                }
            }
            if(!found)
            {
                return false;
            }
        }
        return true;
    }

    void
    parse(String value, java.util.List<java.util.List<RFC2253.RDNPair> > reject,
          java.util.List<java.util.List<RFC2253.RDNPair> > accept)
        throws RFC2253.ParseException
    {
        //
        // Java X500Principal.getName says:
        //
        // If "RFC2253" is specified as the format, this method emits
        // the attribute type keywords defined in RFC 2253 (CN, L, ST,
        // O, OU, C, STREET, DC, UID). Any other attribute type is
        // emitted as an OID. Under a strict reading, RFC 2253 only
        // specifies a UTF-8 string representation. The String
        // returned by this method is the Unicode string achieved by
        // decoding this UTF-8 representation.
        //
        // This means that things like emailAddress and such will be turned into
        // something like:
        //
        // 1.2.840.113549.1.9.1=#160e696e666f407a65726f632e636f6d
        //
        // The left hand side is the OID (see
        // http://www.columbia.edu/~ariel/ssleay/asn1-oids.html) for a
        // list. The right hand side is a BER encoding of the value.
        //
        // This means that the user input, unless it uses the
        // unfriendly OID format, will not directly match the
        // principal.
        //
        // Two possible solutions:
        //
        // Have the RFC2253 parser convert anything that is not CN, L,
        // ST, O, OU, C, STREET, DC, UID into OID format, and have it
        // convert the values into a BER encoding.
        //
        // Send the user data through X500Principal to string form and
        // then through the RFC2253 encoder. This uses the
        // X500Principal to do the encoding for us.
        //
        // The latter is much simpler, however, it means we need to
        // send the data through the parser twice because we split the
        // DNs on ';' which cannot be blindly split because of quotes,
        // \ and such.
        //
        java.util.List<RFC2253.RDNEntry> l = RFC2253.parse(value);
        for(RFC2253.RDNEntry e : l)
        {
            StringBuilder v = new StringBuilder();
            boolean first = true;
            for(RFC2253.RDNPair pair : e.rdn)
            {
                if(!first)
                {
                    v.append(",");
                }
                first = false;
                v.append(pair.key);
                v.append("=");
                v.append(pair.value);
            }
            javax.security.auth.x500.X500Principal princ = new javax.security.auth.x500.X500Principal(v.toString());
            String subjectName = princ.getName(javax.security.auth.x500.X500Principal.RFC2253);
            if(e.negate)
            {
                reject.add(RFC2253.parseStrict(subjectName));
            }
            else
            {
                accept.add(RFC2253.parseStrict(subjectName));
            }
        }
    }

    private static void
    stringify(java.util.List<java.util.List<RFC2253.RDNPair>> matchSet, StringBuilder s)
    {
        boolean addSemi = false;
        for(java.util.List<RFC2253.RDNPair> rdnSet : matchSet)
        {
            if(addSemi)
            {
                s.append(';');
            }
            addSemi = true;
            boolean addComma = false;
            for(RFC2253.RDNPair rdn : rdnSet)
            {
                if(addComma)
                {
                    s.append(',');
                }
                addComma = true;
                s.append(rdn.key);
                s.append('=');
                s.append(rdn.value);
            }
        }
    }

    private Ice.Communicator _communicator;
    private int _traceLevel;

    private java.util.List<java.util.List<RFC2253.RDNPair> > _rejectAll =
        new java.util.LinkedList<java.util.List<RFC2253.RDNPair> >();
    private java.util.List<java.util.List<RFC2253.RDNPair> > _rejectClient =
        new java.util.LinkedList<java.util.List<RFC2253.RDNPair> >();
    private java.util.List<java.util.List<RFC2253.RDNPair> > _rejectAllServer =
        new java.util.LinkedList<java.util.List<RFC2253.RDNPair> >();
    private java.util.Map<String, java.util.List<java.util.List<RFC2253.RDNPair> > > _rejectServer =
        new java.util.HashMap<String, java.util.List<java.util.List<RFC2253.RDNPair> > >();

    private java.util.List<java.util.List<RFC2253.RDNPair> > _acceptAll =
        new java.util.LinkedList<java.util.List<RFC2253.RDNPair> >();
    private java.util.List<java.util.List<RFC2253.RDNPair> > _acceptClient =
        new java.util.LinkedList<java.util.List<RFC2253.RDNPair> >();
    private java.util.List<java.util.List<RFC2253.RDNPair> > _acceptAllServer =
        new java.util.LinkedList<java.util.List<RFC2253.RDNPair> >();
    private java.util.Map<String, java.util.List<java.util.List<RFC2253.RDNPair> > > _acceptServer =
        new java.util.HashMap<String, java.util.List<java.util.List<RFC2253.RDNPair> > >();
}
