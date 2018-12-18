// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Security.Cryptography.X509Certificates;
    using System.Text;

    sealed class TrustManager
    {
        internal TrustManager(Ice.Communicator communicator)
        {
            Debug.Assert(communicator != null);
            _communicator = communicator;
            Ice.Properties properties = communicator.getProperties();
            _traceLevel = properties.getPropertyAsInt("IceSSL.Trace.Security");
            string key = null;
            try
            {
                key = "IceSSL.TrustOnly";
                parse(properties.getProperty(key), _rejectAll, _acceptAll);
                key = "IceSSL.TrustOnly.Client";
                parse(properties.getProperty(key), _rejectClient, _acceptClient);
                key = "IceSSL.TrustOnly.Server";
                parse(properties.getProperty(key), _rejectAllServer, _acceptAllServer);
                Dictionary<string, string> dict = properties.getPropertiesForPrefix("IceSSL.TrustOnly.Server.");
                foreach(KeyValuePair<string, string> entry in dict)
                {
                    key = entry.Key;
                    string name = key.Substring("IceSSL.TrustOnly.Server.".Length);
                    List<List<RFC2253.RDNPair>> reject = new List<List<RFC2253.RDNPair>>();
                    List<List<RFC2253.RDNPair>> accept = new List<List<RFC2253.RDNPair>>();
                    parse(entry.Value, reject, accept);
                    if(reject.Count > 0)
                    {
                        _rejectServer[name] = reject;
                    }
                    if(accept.Count > 0)
                    {
                        _acceptServer[name] = accept;
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

        internal bool verify(IceSSL.ConnectionInfo info, string desc)
        {
            List<List<List<RFC2253.RDNPair>>> reject = new List<List<List<RFC2253.RDNPair>>>(),
                accept = new List<List<List<RFC2253.RDNPair>>>();

            if(_rejectAll.Count != 0)
            {
                reject.Add(_rejectAll);
            }
            if(info.incoming)
            {
                if(_rejectAllServer.Count != 0)
                {
                    reject.Add(_rejectAllServer);
                }
                if(info.adapterName.Length > 0)
                {
                    List<List<RFC2253.RDNPair>> p = null;
                    if(_rejectServer.TryGetValue(info.adapterName, out p))
                    {
                        reject.Add(p);
                    }
                }
            }
            else
            {
                if(_rejectClient.Count != 0)
                {
                    reject.Add(_rejectClient);
                }
            }

            if(_acceptAll.Count != 0)
            {
                accept.Add(_acceptAll);
            }
            if(info.incoming)
            {
                if(_acceptAllServer.Count != 0)
                {
                    accept.Add(_acceptAllServer);
                }
                if(info.adapterName.Length > 0)
                {
                    List<List<RFC2253.RDNPair>> p = null;
                    if(_acceptServer.TryGetValue(info.adapterName, out p))
                    {
                        accept.Add(p);
                    }
                }
            }
            else
            {
                if(_acceptClient.Count != 0)
                {
                    accept.Add(_acceptClient);
                }
            }

            //
            // If there is nothing to match against, then we accept the cert.
            //
            if(reject.Count == 0 && accept.Count == 0)
            {
                return true;
            }

            //
            // If there is no certificate then we match false.
            //
            if(info.certs != null && info.certs.Length > 0)
            {
                X500DistinguishedName subjectDN = info.certs[0].SubjectName;
                string subjectName = subjectDN.Name;
                Debug.Assert(subjectName != null);
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
                                "subject = " + subjectName + "\n" + "adapter = " + info.adapterName + "\n" + desc);
                        }
                        else
                        {
                            _communicator.getLogger().trace("Security", "trust manager evaluating server:\n" +
                                "subject = " + subjectName + "\n" + desc);
                        }
                    }

                    List<RFC2253.RDNPair> dn = RFC2253.parseStrict(subjectName);

                    //
                    // Unescape the DN. Note that this isn't done in
                    // the parser in order to keep the various RFC2253
                    // implementations as close as possible.
                    //
                    for(int i = 0; i < dn.Count; ++i)
                    {
                        RFC2253.RDNPair p = dn[i];
                        p.value = RFC2253.unescape(p.value);
                        dn[i] = p;
                    }

                    //
                    // Fail if we match anything in the reject set.
                    //
                    foreach(List<List<RFC2253.RDNPair>> matchSet in reject)
                    {
                        if(_traceLevel > 0)
                        {
                            StringBuilder s = new StringBuilder("trust manager rejecting PDNs:\n");
                            stringify(matchSet, s);
                            _communicator.getLogger().trace("Security", s.ToString());
                        }
                        if(match(matchSet, dn))
                        {
                            return false;
                        }
                    }

                    //
                    // Succeed if we match anything in the accept set.
                    //
                    foreach(List<List<RFC2253.RDNPair>> matchSet in accept)
                    {
                        if(_traceLevel > 0)
                        {
                            StringBuilder s = new StringBuilder("trust manager accepting PDNs:\n");
                            stringify(matchSet, s);
                            _communicator.getLogger().trace("Security", s.ToString());
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
                return accept.Count == 0;
            }

            return false;
        }

        private bool match(List<List<RFC2253.RDNPair>> matchSet, List<RFC2253.RDNPair> subject)
        {
            foreach(List<RFC2253.RDNPair> item in matchSet)
            {
                if(matchRDNs(item, subject))
                {
                    return true;
                }
            }
            return false;
        }

        private bool matchRDNs(List<RFC2253.RDNPair> match, List<RFC2253.RDNPair> subject)
        {
            foreach(RFC2253.RDNPair matchRDN in match)
            {
                bool found = false;
                foreach(RFC2253.RDNPair subjectRDN in subject)
                {
                    if(matchRDN.key.Equals(subjectRDN.key))
                    {
                        found = true;
                        if(!matchRDN.value.Equals(subjectRDN.value))
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

        // Note that unlike the C++ & Java implementation this returns unescaped data.
        void parse(string value, List<List<RFC2253.RDNPair>> reject, List<List<RFC2253.RDNPair>> accept)
        {
            //
            // As with the Java implementation, the DN that comes from
            // the X500DistinguishedName does not necessarily match
            // the user's input form. Therefore we need to normalize the
            // data to match the C# forms.
            //
            List<RFC2253.RDNEntry> l = RFC2253.parse(value);
            for(int i = 0; i < l.Count; ++i)
            {
                List<RFC2253.RDNPair> dn = l[i].rdn;
                for(int j = 0; j < dn.Count; ++j)
                {
                    RFC2253.RDNPair pair = dn[j];
                    // Normalize the RDN key.
                    if (pair.key == "emailAddress")
                    {
                        pair.key = "E";
                    }
                    else if (pair.key == "ST")
                    {
                        pair.key = "S";
                    }
                    // Unescape the value.
                    pair.value = RFC2253.unescape(pair.value);
                    dn[j] = pair;
                }
                if(l[i].negate)
                {
                    reject.Add(l[i].rdn);
                }
                else
                {
                    accept.Add(l[i].rdn);
                }
            }
        }

        private static void stringify(List<List<RFC2253.RDNPair>> matchSet, StringBuilder s)
        {
            bool addSemi = false;
            foreach(List<RFC2253.RDNPair> rdnSet in matchSet)
            {
                if(addSemi)
                {
                    s.Append(';');
                }
                addSemi = true;
                bool addComma = false;
                foreach(RFC2253.RDNPair rdn in rdnSet)
                {
                    if(addComma)
                    {
                        s.Append(',');
                    }
                    addComma = true;
                    s.Append(rdn.key);
                    s.Append('=');
                    s.Append(rdn.value);
                }
            }
        }

        private Ice.Communicator _communicator;
        private int _traceLevel;

        private List<List<RFC2253.RDNPair>> _rejectAll = new List<List<RFC2253.RDNPair>>();
        private List<List<RFC2253.RDNPair>> _rejectClient = new List<List<RFC2253.RDNPair>>();
        private List<List<RFC2253.RDNPair>> _rejectAllServer = new List<List<RFC2253.RDNPair>>();
        private Dictionary<string, List<List<RFC2253.RDNPair>>> _rejectServer =
            new Dictionary<string, List<List<RFC2253.RDNPair>>>();

        private List<List<RFC2253.RDNPair>> _acceptAll = new List<List<RFC2253.RDNPair>>();
        private List<List<RFC2253.RDNPair>> _acceptClient = new List<List<RFC2253.RDNPair>>();
        private List<List<RFC2253.RDNPair>> _acceptAllServer = new List<List<RFC2253.RDNPair>>();
        private Dictionary<string, List<List<RFC2253.RDNPair>>> _acceptServer =
            new Dictionary<string, List<List<RFC2253.RDNPair>>>();
    }
}
