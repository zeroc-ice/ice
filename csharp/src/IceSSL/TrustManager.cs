//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Security.Cryptography.X509Certificates;
using System.Text;

namespace IceSSL
{
    internal sealed class TrustManager
    {
        internal TrustManager(Ice.Communicator communicator)
        {
            Debug.Assert(communicator != null);
            _communicator = communicator;
            _traceLevel = _communicator.GetPropertyAsInt("IceSSL.Trace.Security") ?? 0;
            string key = "IceSSL.TrustOnly";
            try
            {
                Parse(_communicator.GetProperty(key) ?? "", _rejectAll, _acceptAll);
                key = "IceSSL.TrustOnly.Client";
                Parse(_communicator.GetProperty(key) ?? "", _rejectClient, _acceptClient);
                key = "IceSSL.TrustOnly.Server";
                Parse(_communicator.GetProperty(key) ?? "", _rejectAllServer, _acceptAllServer);
                Dictionary<string, string> dict = _communicator.GetProperties(forPrefix: "IceSSL.TrustOnly.Server.");
                foreach (KeyValuePair<string, string> entry in dict)
                {
                    key = entry.Key;
                    string name = key.Substring("IceSSL.TrustOnly.Server.".Length);
                    var reject = new List<List<RFC2253.RDNPair>>();
                    var accept = new List<List<RFC2253.RDNPair>>();
                    Parse(entry.Value, reject, accept);
                    if (reject.Count > 0)
                    {
                        _rejectServer[name] = reject;
                    }

                    if (accept.Count > 0)
                    {
                        _acceptServer[name] = accept;
                    }
                }
            }
            catch (FormatException ex)
            {
                throw new FormatException($"IceSSL: invalid property `{key};", ex);
            }
        }

        internal bool Verify(IceSSL.ConnectionInfo info, string desc)
        {
            List<List<List<RFC2253.RDNPair>>> reject = new List<List<List<RFC2253.RDNPair>>>(),
                accept = new List<List<List<RFC2253.RDNPair>>>();

            if (_rejectAll.Count != 0)
            {
                reject.Add(_rejectAll);
            }
            if (info.Incoming)
            {
                if (_rejectAllServer.Count != 0)
                {
                    reject.Add(_rejectAllServer);
                }
                if (info.AdapterName!.Length > 0)
                {
                    if (_rejectServer.TryGetValue(info.AdapterName, out List<List<RFC2253.RDNPair>>? p))
                    {
                        reject.Add(p);
                    }
                }
            }
            else
            {
                if (_rejectClient.Count != 0)
                {
                    reject.Add(_rejectClient);
                }
            }

            if (_acceptAll.Count != 0)
            {
                accept.Add(_acceptAll);
            }
            if (info.Incoming)
            {
                if (_acceptAllServer.Count != 0)
                {
                    accept.Add(_acceptAllServer);
                }
                if (info.AdapterName!.Length > 0)
                {
                    if (_acceptServer.TryGetValue(info.AdapterName, out List<List<RFC2253.RDNPair>>? p))
                    {
                        accept.Add(p);
                    }
                }
            }
            else
            {
                if (_acceptClient.Count != 0)
                {
                    accept.Add(_acceptClient);
                }
            }

            //
            // If there is nothing to match against, then we accept the cert.
            //
            if (reject.Count == 0 && accept.Count == 0)
            {
                return true;
            }

            //
            // If there is no certificate then we match false.
            //
            if (info.Certs != null && info.Certs.Length > 0)
            {
                X500DistinguishedName subjectDN = info.Certs[0].SubjectName;
                string subjectName = subjectDN.Name;
                Debug.Assert(subjectName != null);
                try
                {
                    //
                    // Decompose the subject DN into the RDNs.
                    //
                    if (_traceLevel > 0)
                    {
                        if (info.Incoming)
                        {
                            _communicator.Logger.Trace("Security", "trust manager evaluating client:\n" +
                                "subject = " + subjectName + "\n" + "adapter = " + info.AdapterName + "\n" + desc);
                        }
                        else
                        {
                            _communicator.Logger.Trace("Security", "trust manager evaluating server:\n" +
                                "subject = " + subjectName + "\n" + desc);
                        }
                    }

                    List<RFC2253.RDNPair> dn = RFC2253.ParseStrict(subjectName);

                    //
                    // Unescape the DN. Note that this isn't done in
                    // the parser in order to keep the various RFC2253
                    // implementations as close as possible.
                    //
                    for (int i = 0; i < dn.Count; ++i)
                    {
                        RFC2253.RDNPair p = dn[i];
                        p.Value = RFC2253.Unescape(p.Value);
                        dn[i] = p;
                    }

                    //
                    // Fail if we match anything in the reject set.
                    //
                    foreach (List<List<RFC2253.RDNPair>> matchSet in reject)
                    {
                        if (_traceLevel > 0)
                        {
                            var s = new StringBuilder("trust manager rejecting PDNs:\n");
                            Stringify(matchSet, s);
                            _communicator.Logger.Trace("Security", s.ToString());
                        }
                        if (Match(matchSet, dn))
                        {
                            return false;
                        }
                    }

                    //
                    // Succeed if we match anything in the accept set.
                    //
                    foreach (List<List<RFC2253.RDNPair>> matchSet in accept)
                    {
                        if (_traceLevel > 0)
                        {
                            var s = new StringBuilder("trust manager accepting PDNs:\n");
                            Stringify(matchSet, s);
                            _communicator.Logger.Trace("Security", s.ToString());
                        }
                        if (Match(matchSet, dn))
                        {
                            return true;
                        }
                    }
                }
                catch (FormatException e)
                {
                    _communicator.Logger.Warning(
                        $"IceSSL: unable to parse certificate DN `{subjectName}'\nreason: {e.Message}");
                }

                //
                // At this point we accept the connection if there are no explicit accept rules.
                //
                return accept.Count == 0;
            }

            return false;
        }

        private bool Match(List<List<RFC2253.RDNPair>> matchSet, List<RFC2253.RDNPair> subject)
        {
            foreach (List<RFC2253.RDNPair> item in matchSet)
            {
                if (MatchRDNs(item, subject))
                {
                    return true;
                }
            }
            return false;
        }

        private bool MatchRDNs(List<RFC2253.RDNPair> match, List<RFC2253.RDNPair> subject)
        {
            foreach (RFC2253.RDNPair matchRDN in match)
            {
                bool found = false;
                foreach (RFC2253.RDNPair subjectRDN in subject)
                {
                    if (matchRDN.Key.Equals(subjectRDN.Key))
                    {
                        found = true;
                        if (!matchRDN.Value.Equals(subjectRDN.Value))
                        {
                            return false;
                        }
                    }
                }
                if (!found)
                {
                    return false;
                }
            }
            return true;
        }

        // Note that unlike the C++ & Java implementation this returns unescaped data.
        private void Parse(string value, List<List<RFC2253.RDNPair>> reject, List<List<RFC2253.RDNPair>> accept)
        {
            //
            // As with the Java implementation, the DN that comes from
            // the X500DistinguishedName does not necessarily match
            // the user's input form. Therefore we need to normalize the
            // data to match the C# forms.
            //
            List<RFC2253.RDNEntry> l = RFC2253.Parse(value);
            for (int i = 0; i < l.Count; ++i)
            {
                List<RFC2253.RDNPair> dn = l[i].Rdn;
                for (int j = 0; j < dn.Count; ++j)
                {
                    RFC2253.RDNPair pair = dn[j];
                    // Normalize the RDN key.
                    if (pair.Key == "emailAddress")
                    {
                        pair.Key = "E";
                    }
                    else if (pair.Key == "ST")
                    {
                        pair.Key = "S";
                    }
                    // Unescape the value.
                    pair.Value = RFC2253.Unescape(pair.Value);
                    dn[j] = pair;
                }
                if (l[i].Negate)
                {
                    reject.Add(l[i].Rdn);
                }
                else
                {
                    accept.Add(l[i].Rdn);
                }
            }
        }

        private static void Stringify(List<List<RFC2253.RDNPair>> matchSet, StringBuilder s)
        {
            bool addSemi = false;
            foreach (List<RFC2253.RDNPair> rdnSet in matchSet)
            {
                if (addSemi)
                {
                    s.Append(';');
                }
                addSemi = true;
                bool addComma = false;
                foreach (RFC2253.RDNPair rdn in rdnSet)
                {
                    if (addComma)
                    {
                        s.Append(',');
                    }
                    addComma = true;
                    s.Append(rdn.Key);
                    s.Append('=');
                    s.Append(rdn.Value);
                }
            }
        }

        private readonly Ice.Communicator _communicator;
        private readonly int _traceLevel;

        private readonly List<List<RFC2253.RDNPair>> _rejectAll = new List<List<RFC2253.RDNPair>>();
        private readonly List<List<RFC2253.RDNPair>> _rejectClient = new List<List<RFC2253.RDNPair>>();
        private readonly List<List<RFC2253.RDNPair>> _rejectAllServer = new List<List<RFC2253.RDNPair>>();
        private readonly Dictionary<string, List<List<RFC2253.RDNPair>>> _rejectServer =
            new Dictionary<string, List<List<RFC2253.RDNPair>>>();

        private readonly List<List<RFC2253.RDNPair>> _acceptAll = new List<List<RFC2253.RDNPair>>();
        private readonly List<List<RFC2253.RDNPair>> _acceptClient = new List<List<RFC2253.RDNPair>>();
        private readonly List<List<RFC2253.RDNPair>> _acceptAllServer = new List<List<RFC2253.RDNPair>>();
        private readonly Dictionary<string, List<List<RFC2253.RDNPair>>> _acceptServer =
            new Dictionary<string, List<List<RFC2253.RDNPair>>>();
    }
}
