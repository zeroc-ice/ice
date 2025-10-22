// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.SSL;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationException;
import com.zeroc.Ice.ParseException;
import com.zeroc.Ice.Properties;

import java.security.cert.X509Certificate;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import javax.security.auth.x500.X500Principal;

class TrustManager {
    TrustManager(Communicator communicator) {
        assert communicator != null;
        _communicator = communicator;
        Properties properties = communicator.getProperties();
        _traceLevel = properties.getIcePropertyAsInt("IceSSL.Trace.Security");
        String key = null;
        try {
            key = "IceSSL.TrustOnly";
            parse(properties.getProperty(key), _rejectAll, _acceptAll);
            key = "IceSSL.TrustOnly.Client";
            parse(properties.getProperty(key), _rejectClient, _acceptClient);
            key = "IceSSL.TrustOnly.Server";
            parse(properties.getProperty(key), _rejectAllServer, _acceptAllServer);
            Map<String, String> dict = properties.getPropertiesForPrefix("IceSSL.TrustOnly.Server.");

            for (Map.Entry<String, String> p : dict.entrySet()) {
                key = p.getKey();
                String name = key.substring("IceSSL.TrustOnly.Server.".length());
                List<List<RFC2253.RDNPair>> reject = new LinkedList<>();
                List<List<RFC2253.RDNPair>> accept = new LinkedList<>();
                parse(p.getValue(), reject, accept);
                if (!reject.isEmpty()) {
                    _rejectServer.put(name, reject);
                }
                if (!accept.isEmpty()) {
                    _acceptServer.put(name, accept);
                }
            }
        } catch (ParseException ex) {
            throw new InitializationException("Ice.SSL: invalid property " + key, ex);
        }
    }

    boolean verify(ConnectionInfo info, String desc) {
        List<List<List<RFC2253.RDNPair>>> reject = new LinkedList<>();
        List<List<List<RFC2253.RDNPair>>> accept = new LinkedList<>();

        if (!_rejectAll.isEmpty()) {
            reject.add(_rejectAll);
        }
        if (info.incoming) {
            if (!_rejectAllServer.isEmpty()) {
                reject.add(_rejectAllServer);
            }
            if (!info.adapterName.isEmpty()) {
                List<List<RFC2253.RDNPair>> p = _rejectServer.get(info.adapterName);
                if (p != null) {
                    reject.add(p);
                }
            }
        } else {
            if (!_rejectClient.isEmpty()) {
                reject.add(_rejectClient);
            }
        }

        if (!_acceptAll.isEmpty()) {
            accept.add(_acceptAll);
        }
        if (info.incoming) {
            if (!_acceptAllServer.isEmpty()) {
                accept.add(_acceptAllServer);
            }
            if (!info.adapterName.isEmpty()) {
                List<List<RFC2253.RDNPair>> p = _acceptServer.get(info.adapterName);
                if (p != null) {
                    accept.add(p);
                }
            }
        } else {
            if (!_acceptClient.isEmpty()) {
                accept.add(_acceptClient);
            }
        }

        // If there is nothing to match against, then we accept the cert.
        if (reject.isEmpty() && accept.isEmpty()) {
            return true;
        }

        // If there is no certificate then we match false.
        if (info.certs != null && info.certs.length > 0) {
            X500Principal subjectDN = ((X509Certificate) info.certs[0]).getSubjectX500Principal();
            String subjectName = subjectDN.getName(X500Principal.RFC2253);
            assert subjectName != null;
            try {
                // Decompose the subject DN into the RDNs.
                if (_traceLevel > 0) {
                    if (info.incoming) {
                        _communicator
                            .getLogger()
                            .trace(
                                "Security",
                                "trust manager evaluating client:\n"
                                    + "subject = "
                                    + subjectName
                                    + "\n"
                                    + "adapter = "
                                    + info.adapterName
                                    + "\n"
                                    + desc);
                    } else {
                        _communicator
                            .getLogger()
                            .trace(
                                "Security",
                                "trust manager evaluating server:\n"
                                    + "subject = "
                                    + subjectName
                                    + "\n"
                                    + desc);
                    }
                }
                List<RFC2253.RDNPair> dn = RFC2253.parseStrict(subjectName);

                // Fail if we match anything in the reject set.
                for (List<List<RFC2253.RDNPair>> matchSet : reject) {
                    if (_traceLevel > 1) {
                        StringBuilder s = new StringBuilder("trust manager rejecting PDNs:\n");
                        stringify(matchSet, s);
                        _communicator.getLogger().trace("Security", s.toString());
                    }
                    if (match(matchSet, dn)) {
                        return false;
                    }
                }

                // Succeed if we match anything in the accept set.
                for (List<List<RFC2253.RDNPair>> matchSet : accept) {
                    if (_traceLevel > 1) {
                        StringBuilder s = new StringBuilder("trust manager accepting PDNs:\n");
                        stringify(matchSet, s);
                        _communicator.getLogger().trace("Security", s.toString());
                    }
                    if (match(matchSet, dn)) {
                        return true;
                    }
                }
            } catch (ParseException ex) {
                String m = "Ice.SSL: unable to parse certificate DN `" + subjectName + "'\nreason: " + ex.getMessage();
                _communicator.getLogger().warning(m);
            }

            // At this point we accept the connection if there are no explicit accept rules.
            return accept.isEmpty();
        }

        return false;
    }

    private boolean match(List<List<RFC2253.RDNPair>> matchSet, List<RFC2253.RDNPair> subject) {
        for (List<RFC2253.RDNPair> r : matchSet) {
            if (matchRDNs(r, subject)) {
                return true;
            }
        }
        return false;
    }

    private boolean matchRDNs(List<RFC2253.RDNPair> match, List<RFC2253.RDNPair> subject) {
        for (RFC2253.RDNPair matchRDN : match) {
            boolean found = false;
            for (RFC2253.RDNPair subjectRDN : subject) {
                if (matchRDN.key.equals(subjectRDN.key)) {
                    found = true;
                    if (!matchRDN.value.equals(subjectRDN.value)) {
                        return false;
                    }
                }
            }
            if (!found) {
                return false;
            }
        }
        return true;
    }

    void parse(String value, List<List<RFC2253.RDNPair>> reject, List<List<RFC2253.RDNPair>> accept)
        throws ParseException {
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
        List<RFC2253.RDNEntry> l = RFC2253.parse(value);
        for (RFC2253.RDNEntry e : l) {
            StringBuilder v = new StringBuilder();
            boolean first = true;
            for (RFC2253.RDNPair pair : e.rdn) {
                if (!first) {
                    v.append(',');
                }
                first = false;
                v.append(pair.key);
                v.append('=');
                v.append(pair.value);
            }
            var principal = new X500Principal(v.toString());
            String subjectName = principal.getName(X500Principal.RFC2253);
            if (e.negate) {
                reject.add(RFC2253.parseStrict(subjectName));
            } else {
                accept.add(RFC2253.parseStrict(subjectName));
            }
        }
    }

    private static void stringify(List<List<RFC2253.RDNPair>> matchSet, StringBuilder s) {
        boolean addSemi = false;
        for (List<RFC2253.RDNPair> rdnSet : matchSet) {
            if (addSemi) {
                s.append(';');
            }
            addSemi = true;
            boolean addComma = false;
            for (RFC2253.RDNPair rdn : rdnSet) {
                if (addComma) {
                    s.append(',');
                }
                addComma = true;
                s.append(rdn.key);
                s.append('=');
                s.append(rdn.value);
            }
        }
    }

    private final Communicator _communicator;
    private final int _traceLevel;

    private final List<List<RFC2253.RDNPair>> _rejectAll = new LinkedList<>();
    private final List<List<RFC2253.RDNPair>> _rejectClient = new LinkedList<>();
    private final List<List<RFC2253.RDNPair>> _rejectAllServer = new LinkedList<>();
    private final Map<String, List<List<RFC2253.RDNPair>>> _rejectServer = new HashMap<>();

    private final List<List<RFC2253.RDNPair>> _acceptAll = new LinkedList<>();
    private final List<List<RFC2253.RDNPair>> _acceptClient = new LinkedList<>();
    private final List<List<RFC2253.RDNPair>> _acceptAllServer = new LinkedList<>();
    private final Map<String, List<List<RFC2253.RDNPair>>> _acceptServer = new HashMap<>();
}
