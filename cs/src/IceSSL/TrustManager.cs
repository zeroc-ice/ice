// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    using System;
    using System.Collections;
    using System.Diagnostics;
    using System.Security.Cryptography.X509Certificates;

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
	        all_ = parse(properties.getProperty(key));
	        key = "IceSSL.TrustOnly.Client";
	        client_ = parse(properties.getProperty(key));
	        key = "IceSSL.TrustOnly.Server";
	        allServer_ = parse(properties.getProperty(key));
	        Ice.PropertyDict dict = properties.getPropertiesForPrefix("IceSSL.TrustOnly.Server.");
                foreach (DictionaryEntry entry in dict)
                {
		    string dkey = (string)entry.Key;
		    string dname = dkey.Substring("IceSSL.TrustOnly.Server.".Length);
                    server_[dname] = parse((string)entry.Value);
	        }
	    }
	    catch(RFC2253.ParseException e)
	    {
	        Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
	        ex.reason = "IceSSL: invalid property " + key  + ":\n" + e.reason;
	        throw ex;
	    }
        }

        internal bool
        verify(ConnectionInfo info)
        {
	    ArrayList trustset = new ArrayList();
	    if(all_.Count != 0)
	    {
	        trustset.Add(all_);
	    }

	    if(info.incoming)
	    {
	        if(allServer_.Count != 0)
	        {
		    trustset.Add(allServer_);
	        }
	        if(info.adapterName.Length > 0)
	        {
                    ArrayList p = (ArrayList)server_[info.adapterName];
		    if(p != null)
		    {
		        trustset.Add(p);
		    }
	        }
	    }
	    else
	    {
	        if(client_.Count != 0)
	        {
		    trustset.Add(client_);
	        }
	    }

	    //
	    // If there is nothing to match against, then we accept the cert.
	    //
	    if(trustset.Count == 0)
	    {
	        return true;
	    }

	    //
	    // If there is no certificate then we match false.
	    //
	    if(info.certs.Length != 0)
	    {
	        X500DistinguishedName subjectDN = info.certs[0].SubjectName;
                string subjectName = subjectDN.Name;
                Debug.Assert(subjectName != null);
	        try
	        {
		    //
		    // Decompose the subject DN into the RDNs.
		    //
		    if(traceLevel_ > 0)
		    {
		        communicator_.getLogger().trace("Security", "trust manager evaluating peer DN:\n" +
							subjectName);
		    }

		    ArrayList dn = RFC2253.parseStrict(subjectName);
                    
                    //
                    // Unescape the DN. Note that this isn't done in
                    // the parser in order to keep the various RFC2253
                    // implementations as close as possible.
                    //
                    for (int i = 0; i < dn.Count; ++i)
                    {
                        RFC2253.RDNPair p = (RFC2253.RDNPair)dn[i];
                        p.value = RFC2253.unescape(p.value);
                        dn[i] = p;
                    }

		    //
		    // Try matching against everything in the trust set.
		    //
                    for(int i = 0; i < trustset.Count; ++i)
		    {
		        if(match((ArrayList)trustset[i], dn))
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
	    }

	    return false;
        }

        private bool
        match(ArrayList matchSet, ArrayList subject)
        {
            for(int i = 0; i < matchSet.Count; ++i)
            {
	        if(matchRDNs((ArrayList)matchSet[i], subject))
	        {
		    return true;
	        }
	    }
	    return false;
        }

        private bool
        matchRDNs(ArrayList match, ArrayList subject)
        {
            for(int i = 0; i < match.Count; ++i)
	    {
                RFC2253.RDNPair matchRDN = (RFC2253.RDNPair)match[i];
                bool found = false;
                
                for(int j = 0; j < subject.Count; ++j)
	        {
		    RFC2253.RDNPair subjectRDN = (RFC2253.RDNPair)subject[j];
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
        ArrayList
        parse(string value)
        {
	    //
            // As with the Java implementation, the DN that comes from
            // the X500DistinguishedName does not necessarily match
            // the user's input form. Therefore we need to normalize the
            // data to match the C# forms.
            //
            ArrayList l = RFC2253.parse(value);
            for(int i = 0; i < l.Count; ++i)
	    {
	        ArrayList dn = (ArrayList)l[i];
                for(int j = 0; j < dn.Count; ++j)
	        {
		    RFC2253.RDNPair pair = (RFC2253.RDNPair)dn[j];
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
            }
	    return l;
        }

        private Ice.Communicator communicator_;
        private int traceLevel_;

        private ArrayList all_;
        private ArrayList client_;
        private ArrayList allServer_;
        private Hashtable server_ = new Hashtable();
    }
}
