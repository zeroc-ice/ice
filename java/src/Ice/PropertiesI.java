// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package Ice;

final class PropertiesI extends LocalObjectImpl implements Properties
{
    public synchronized String
    getProperty(String key)
    {
        String result = (String)_properties.get(key);
        if(result == null)
        {
            result = System.getProperty(key);
        }
        if(result == null)
        {
            result = "";
        }
        return result;
    }

    public synchronized String
    getPropertyWithDefault(String key, String value)
    {
        String result = (String)_properties.get(key);
        if(result == null)
        {
            result = System.getProperty(key);
        }
        if(result == null)
        {
            result = value;
        }
        return result;
    }

    public int
    getPropertyAsInt(String key)
    {
	return getPropertyAsIntWithDefault(key, 0);
    }

    public synchronized int
    getPropertyAsIntWithDefault(String key, int value)
    {
        String result = (String)_properties.get(key);
        if(result == null)
        {
            result = System.getProperty(key);
        }
        if(result == null)
        {
            return value;
        }

	try
	{
	    return Integer.parseInt(result);
	}
	catch(NumberFormatException ex)
	{
	    return 0;
	}
    }

    public synchronized java.util.Map
    getPropertiesForPrefix(String prefix)
    {
	java.util.HashMap result = new java.util.HashMap();
        java.util.Iterator p = _properties.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            String key = (String)entry.getKey();
            String value = (String)entry.getValue();
            if(prefix.length() == 0 || key.startsWith(prefix))
            {
		result.put(key, value);
            }
        }
	return result;
    }

    public synchronized void
    setProperty(String key, String value)
    {
	//
	// Check if the property is legal. (We write to System.err instead of using
	// a logger because no logger may be established at the time the property
	// is parsed.)
	//
        int dotPos = key.indexOf('.');
	if(dotPos != -1)
	{
	    String prefix = key.substring(0, dotPos);
	    String[] validProps = (String[])_vp._validProps.get(prefix);
	    if(validProps != null)
	    {
	        String suffix = key.substring(dotPos + 1, key.length());
		boolean found = false;
		for(int j = 0; j < validProps.length && !found; ++j)
		{
		    int starPos = validProps[j].indexOf('*');
		    if(starPos == -1)
		    {
		        found = validProps[j].equals(suffix);
		    }
		    else
		    {
		        int max = java.lang.Math.min(suffix.length(), starPos);
			String s1 = suffix.substring(0, max);
			String s2 = validProps[j].substring(0, max);
			found = s1.equals(s2);
		    }
		}
		if(!found)
		{
		    System.err.println("warning: unknown property: " + key);
		}
	    }
	}

	//
	// Set or clear the property.
	//
	if(key != null && key.length() > 0)
	{
	    if(value != null && value.length() > 0)
	    {
		_properties.put(key, value);
	    }
	    else
	    {
		_properties.remove(key);
	    }
	}
    }

    public synchronized String[]
    getCommandLineOptions()
    {
        String[] result = new String[_properties.size()];
        java.util.Iterator p = _properties.entrySet().iterator();
        int i = 0;
        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            result[i++] = "--" + entry.getKey() + "=" + entry.getValue();
        }
        assert(i == result.length);
        return result;
    }

    public synchronized String[]
    parseCommandLineOptions(String pfx, String[] options)
    {
	String prefix = pfx;
	if(pfx.length() > 0 && pfx.charAt(pfx.length() - 1) != '.')
	{
	    pfx += '.';
	}
	pfx = "--" + pfx;

        java.util.ArrayList result = new java.util.ArrayList();
        for(int i = 0; i < options.length; i++)
        {
            String opt = options[i];
            if(opt.startsWith(pfx))
            {
                if(opt.indexOf('=') == -1)
                {
                    opt += "=1";
                }

                parseLine(opt.substring(2));
            }
            else
            {
                result.add(opt);
            }
        }
        String[] arr = new String[result.size()];
        result.toArray(arr);
        return arr;
    }

    public synchronized void
    load(String file)
    {
        try
        {
            java.io.FileReader fr = new java.io.FileReader(file);
            java.io.BufferedReader br = new java.io.BufferedReader(fr);
            parse(br);
        }
        catch(java.io.IOException ex)
        {
            SyscallException se = new SyscallException();
            se.initCause(ex); // Exception chaining
            throw se;
        }
    }

    public synchronized Properties
    _clone()
    {
        PropertiesI p = new PropertiesI(new String[0]);
        p._properties.putAll(_properties);
        return p;
    }

    PropertiesI()
    {
    }

    PropertiesI(String[] args)
    {
        for(int i = 0; i < args.length; i++)
        {
            if(args[i].startsWith("--Ice.Config"))
            {
                String line = args[i];
                if(line.indexOf('=') == -1)
                {
                    line += "=1";
                }
                parseLine(line.substring(2));
            }
        }

        loadConfig();
    }

    PropertiesI(StringSeqHolder args)
    {
        for(int i = 0; i < args.value.length; i++)
        {
            if(args.value[i].startsWith("--Ice.Config"))
            {
                String line = args.value[i];
                if(line.indexOf('=') == -1)
                {
                    line += "=1";
                }
                parseLine(line.substring(2));
                String[] arr = new String[args.value.length - 1];
                System.arraycopy(args.value, 0, arr, 0, i);
                if(i < args.value.length - 1)
                {
                    System.arraycopy(args.value, i + 1, arr, i, args.value.length - i - 1);
                }
                args.value = arr;
            }
        }

        loadConfig();
    }

    private void
    parse(java.io.BufferedReader in)
    {
        try
        {
            String line;
            while((line = in.readLine()) != null)
            {
                parseLine(line);
            }
        }
        catch(java.io.IOException ex)
        {
            SyscallException se = new SyscallException();
            se.initCause(ex); // Exception chaining
            throw se;
        }
    }

    private void
    parseLine(String line)
    {
        String s = line;

        int hash = s.indexOf('#');
        if(hash == 0)
        {
            return; // ignore comment lines
        }
        else if(hash != -1)
        {
            s = s.substring(0, hash);
        }

        s = s.trim();

        final char[] arr = s.toCharArray();
        int end = -1;
        for(int i = 0; i < arr.length; i++)
        {
            if(arr[i] == ' ' || arr[i] == '\t' || arr[i] == '\r' || arr[i] == '\n' || arr[i] == '=')
            {
                end = i;
                break;
            }
        }
        if(end == -1)
        {
            return;
        }

        String key = s.substring(0, end);

        end = s.indexOf('=', end);
        if(end == -1)
        {
            return;
        }
        ++end;

        String value = "";
        if(end < s.length())
        {
            value = s.substring(end).trim();
        }

        setProperty(key, value);
    }

    private void
    loadConfig()
    {
        String value = getProperty("Ice.Config");

        if(value.equals("1"))
        {
            value = "";
        }

        if(value.length() > 0)
        {
            String[] files = value.split(",");
            for(int i = 0; i < files.length; i++)
            {
                load(files[i]);
            }
        }

        setProperty("Ice.Config", value);
    }

    private java.util.HashMap _properties = new java.util.HashMap();

//
// Valid properties for each application.
// A '*' character is a wildcard. If used, it must appear at the end of the string.
// Examples: "Ice.Foo.*" allows all properties with that prefix, such as "Ice.Foo.Bar".
//           "Ice.Foo*" allows properties such as "Ice.Foo.Bar" and "Ice.FooBar".
//
    private static final String _iceProps[] = {
						    "ChangeUser",
						    "Config",
						    "ConnectionIdleTime",
						    "Daemon",
						    "DaemonNoChdir",
						    "DaemonNoClose",
						    "Default.Host",
						    "Default.Locator",
						    "Default.Protocol",
						    "Default.Router",
						    "Logger.Timestamp",
						    "MonitorConnections",
						    "Nohup",
						    "NullHandleAbort",
						    "Override.Compress",
						    "Override.Timeout",
						    "Plugin.*",
						    "PrintAdapterReady",
						    "PrintProcessId",
						    "ProgramName",
						    "RetryIntervals",
						    "ServerIdleTime",
						    "ThreadPool.Client.Size",
						    "ThreadPool.Client.SizeMax",
						    "ThreadPool.Client.SizeWarn",
						    "ThreadPool.Server.Size",
						    "Trace.Network",
						    "Trace.Protocol",
						    "Trace.Retry",
						    "Trace.Slicing",
						    "UseSyslog",
						    "Warn.AMICallback",
						    "Warn.Connections",
						    "Warn.Dispatch",
						    "Warn.Leaks"
				              };

    private static final String _iceBoxProps[] = {
						    "DBEnvName.*",
						    "PrintServicesReady",
						    "Service.*",
						    "ServiceManager.AdapterId",
						    "ServiceManager.Endpoints",
						    "ServiceManager.Identity",
						    "UseSharedCommunicator.*"
					         };

    private static final String _icePackProps[] = {
						    "Node.AdapterId",
						    "Node.CollocateRegistry",
						    "Node.Data",
						    "Node.Endpoints",
						    "Node.Name",
						    "Node.PrintServersReady",
						    "Node.PropertiesOverride",
						    "Node.ThreadPool.Size",
						    "Node.Trace.Activator",
						    "Node.Trace.Adapter",
						    "Node.Trace.Server",
						    "Node.WaitTime",
						    "Registry.Admin.AdapterId",
						    "Registry.Admin.Endpoints",
						    "Registry.Client.Endpoints",
						    "Registry.Data",
						    "Registry.DynamicRegistration",
						    "Registry.Internal.AdapterId",
						    "Registry.Internal.Endpoints",
						    "Registry.Server.Endpoints",
						    "Registry.Trace.AdapterRegistry",
						    "Registry.Trace.NodeRegistry",
						    "Registry.Trace.ObjectRegistry",
						    "Registry.Trace.ServerRegistry"
					          };

    private static final String _icePatchProps[] = {
						    "BusyTimeout",
						    "RemoveOrphaned",
						    "Thorough",
						    "Trace.Files",
						    "UpdatePeriod"
						   };

    private static final String _iceSSLProps[] = {
						    "Client.CertificateVerifier",
						    "Client.CertPath*",
						    "Client.Config",
						    "Client.Handshake.Retries",
						    "Client.Overrides.CACertificate",
						    "Client.Overrides.DSA.Certificate",
						    "Client.Overrides.DSA.PrivateKey",
						    "Client.Overrides.RSA.Certificate",
						    "Client.Overrides.RSA.PrivateKey",
						    "Client.Passphrase.Retries",
						    "Server.CertificateVerifier",
						    "Server.CertPath*",
						    "Server.Config",
						    "Server.Overrides.CACertificate",
						    "Server.Overrides.DSA.Certificate",
						    "Server.Overrides.DSA.PrivateKey",
						    "Server.Overrides.RSA.Certificate",
						    "Server.Overrides.RSA.PrivateKey",
						    "Server.Passphrase.Retries",
						    "Trace.Security"
					         };

    private static final String _iceStormProps[] = {
						    "Flush.Timeout",
						    "Publish.Endpoints",
						    "TopicManager.Endpoints",
						    "TopicManager.Proxy",
						    "Trace.Flush",
						    "Trace.Subscriber",
						    "Trace.Topic",
						    "Trace.TopicManager"
					           };

    private static final String _glacierProps[] = {
						    "Router.AcceptCert",
						    "Router.AllowCategories",
						    "Router.Client.BatchSleepTime",
						    "Router.Client.Endpoints",
						    "Router.Client.ForwardContext",
						    "Router.Endpoints",
						    "Router.Identity",
						    "Router.PrintProxyOnFd",
						    "Router.Server.BatchSleepTime",
						    "Router.Server.Endpoints",
						    "Router.Server.ForwardContext",
						    "Router.SessionManager",
						    "Router.Trace.Client",
						    "Router.Trace.RoutingTable",
						    "Router.Trace.Server",
						    "Router.UserId",
						    "Starter.AddUserToAllowCategories",
						    "Starter.Certificate.BitStrength",
						    "Starter.Certificate.CommonName",
						    "Starter.Certificate.Country",
						    "Starter.Certificate.IssuedAdjust",
						    "Starter.Certificate.Locality",
						    "Starter.Certificate.Organization",
						    "Starter.Certificate.OrganizationalUnit",
						    "Starter.Certificate.SecondsValid",
						    "Starter.Certificate.StateProvince",
						    "Starter.CryptPasswords",
						    "Starter.Endpoints",
						    "Starter.PasswordVerifier",
						    "Starter.PropertiesOverride",
						    "Starter.RouterPath",
						    "Starter.StartupTimeout",
						    "Starter.Trace"
					          };

    private static final String _freezeProps[] = {
						    "Trace.DB",
						    "Trace.Evictor"
					         };

    private static final class ValidProps
    {
        ValidProps()
	{
	    _validProps.put("Freeze", _freezeProps);
	    _validProps.put("Glacier", _glacierProps);
	    _validProps.put("IceBox", _iceBoxProps);
	    _validProps.put("Ice", _iceProps);
	    _validProps.put("IcePack", _icePackProps);
	    _validProps.put("IcePatch", _icePatchProps);
	    _validProps.put("IceSSL", _iceSSLProps);
	    _validProps.put("IceStorm", _iceStormProps);
	}

        public static java.util.HashMap _validProps = new java.util.HashMap();
    }

    private static final ValidProps _vp = new ValidProps();
}
