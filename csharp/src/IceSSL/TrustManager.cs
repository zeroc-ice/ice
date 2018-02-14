// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Security.Cryptography.X509Certificates;
    using System.Text;

    sealed class TrustManager
    {
        internal TrustManager(Ice.Communicator communicator)
        {
            Debug.Assert(communicator != null);
            communicator_ = communicator;
            Ice.Properties properties = communicator.getProperties();
            traceLevel_ = properties.getPropertyAsInt("IceSSL.Trace.Security");
            string key = null;
            try
            {
                key = "IceSSL.TrustOnly";
                parse(properties.getProperty(key), rejectAll_, acceptAll_);
                key = "IceSSL.TrustOnly.Client";
                parse(properties.getProperty(key), rejectClient_, acceptClient_);
                key = "IceSSL.TrustOnly.Server";
                parse(properties.getProperty(key), rejectAllServer_, acceptAllServer_);
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
                        rejectServer_[name] = reject;
                    }
                    if(accept.Count > 0)
                    {
                        acceptServer_[name] = accept;
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

        internal bool verify(NativeConnectionInfo info)
        {
            List<List<List<RFC2253.RDNPair>>> reject = new List<List<List<RFC2253.RDNPair>>>(),
                accept = new List<List<List<RFC2253.RDNPair>>>();

            if(rejectAll_.Count != 0)
            {
                reject.Add(rejectAll_);
            }
            if(info.incoming)
            {
                if(rejectAllServer_.Count != 0)
                {
                    reject.Add(rejectAllServer_);
                }
                if(info.adapterName.Length > 0)
                {
                    List<List<RFC2253.RDNPair>> p = null;
                    if(rejectServer_.TryGetValue(info.adapterName, out p))
                    {
                        reject.Add(p);
                    }
                }
            }
            else
            {
                if(rejectClient_.Count != 0)
                {
                    reject.Add(rejectClient_);
                }
            }

            if(acceptAll_.Count != 0)
            {
                accept.Add(acceptAll_);
            }
            if(info.incoming)
            {
                if(acceptAllServer_.Count != 0)
                {
                    accept.Add(acceptAllServer_);
                }
                if(info.adapterName.Length > 0)
                {
                    List<List<RFC2253.RDNPair>> p = null;
                    if(acceptServer_.TryGetValue(info.adapterName, out p))
                    {
                        accept.Add(p);
                    }
                }
            }
            else
            {
                if(acceptClient_.Count != 0)
                {
                    accept.Add(acceptClient_);
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
            if(info.nativeCerts != null && info.nativeCerts.Length > 0)
            {
#if UNITY
                throw new Ice.FeatureNotSupportedException("certificate subjectName not available");
#else
                X500DistinguishedName subjectDN = info.nativeCerts[0].SubjectName;
                string subjectName = subjectDN.Name;
                Debug.Assert(subjectName != null);
                try
                {
                    //
                    // Decompose the subject DN into the RDNs.
                    //
                    if(traceLevel_ > 0)
                    {
                        if(info.incoming)
                        {
                            communicator_.getLogger().trace("Security", "trust manager evaluating client:\n" +
                                "subject = " + subjectName + "\n" +
                                "adapter = " + info.adapterName + "\n" +
                                "local addr = " + info.localAddress + ":" + info.localPort + "\n" +
                                "remote addr = " +  info.remoteAddress + ":" + info.remotePort);
                        }
                        else
                        {
                            communicator_.getLogger().trace("Security", "trust manager evaluating server:\n" +
                                "subject = " + subjectName + "\n" +
                                "local addr = " + info.localAddress + ":" + info.localPort + "\n" +
                                "remote addr = " +  info.remoteAddress + ":" + info.remotePort);
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
                        if(traceLevel_ > 0)
                        {
                            StringBuilder s = new StringBuilder("trust manager rejecting PDNs:\n");
                            stringify(matchSet, s);
                            communicator_.getLogger().trace("Security", s.ToString());
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
                        if(traceLevel_ > 0)
                        {
                            StringBuilder s = new StringBuilder("trust manager accepting PDNs:\n");
                            stringify(matchSet, s);
                            communicator_.getLogger().trace("Security", s.ToString());
                        }
                        if(match(matchSet, dn))
                        {
                            return true;
                        }
                    }
                }
                catch(RFC2253.ParseException e)
                {
                    communicator_.getLogger().warning(
                        "IceSSL: unable to parse certificate DN `" + subjectName + "'\nreason: " + e.reason);
                }

                //
                // At this point we accept the connection if there are no explicit accept rules.
                //
                return accept.Count == 0;
#endif
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

        private Ice.Communicator communicator_;
        private int traceLevel_;

        private List<List<RFC2253.RDNPair>> rejectAll_ = new List<List<RFC2253.RDNPair>>();
        private List<List<RFC2253.RDNPair>> rejectClient_ = new List<List<RFC2253.RDNPair>>();
        private List<List<RFC2253.RDNPair>> rejectAllServer_ = new List<List<RFC2253.RDNPair>>();
        private Dictionary<string, List<List<RFC2253.RDNPair>>> rejectServer_ =
            new Dictionary<string, List<List<RFC2253.RDNPair>>>();

        private List<List<RFC2253.RDNPair>> acceptAll_ = new List<List<RFC2253.RDNPair>>();
        private List<List<RFC2253.RDNPair>> acceptClient_ = new List<List<RFC2253.RDNPair>>();
        private List<List<RFC2253.RDNPair>> acceptAllServer_ = new List<List<RFC2253.RDNPair>>();
        private Dictionary<string, List<List<RFC2253.RDNPair>>> acceptServer_ =
            new Dictionary<string, List<List<RFC2253.RDNPair>>>();
    }
}
