// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class PropertiesI extends LocalObjectImpl implements Properties
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

    public synchronized String[]
    parseIceCommandLineOptions(String[] options)
    {
        String[] args = parseCommandLineOptions("Ice", options);
        args = parseCommandLineOptions("Freeze", args);
        args = parseCommandLineOptions("Glacier", args);
        args = parseCommandLineOptions("Glacier2", args);
        args = parseCommandLineOptions("IceBox", args);
        args = parseCommandLineOptions("IcePack", args);
        args = parseCommandLineOptions("IcePatch", args);
        args = parseCommandLineOptions("IceSSL", args);
        args = parseCommandLineOptions("IceStorm", args);
        return args;
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
        return new PropertiesI(this);
    }

    PropertiesI(PropertiesI p)
    {
        _properties.putAll(p._properties);
    }

    PropertiesI()
    {
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

	args.value = parseIceCommandLineOptions(args.value);
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
    private static final String _iceProps[] =
    {
	"ChangeUser",
	"Config",
	"ConnectionIdleTime",
	"Default.Host",
	"Default.Locator",
	"Default.Package",
	"Default.Protocol",
	"Default.Router",
	"GC.Interval",
	"Logger.Timestamp",
	"MessageSizeMax",
	"MonitorConnections",
	"Nohup",
	"NullHandleAbort",
	"Override.Compress",
	"Override.ConnectTimeout",
	"Override.Timeout",
	"Package.*",
	"Plugin.*",
	"PrintAdapterReady",
	"PrintProcessId",
	"ProgramName",
	"RetryIntervals",
	"StdErr",
	"StdOut",
	"ServerId",
	"ServerIdleTime",
	"ThreadPool.Client.Size",
	"ThreadPool.Client.SizeMax",
	"ThreadPool.Client.SizeWarn",
	"ThreadPool.Client.StackSize",
	"ThreadPool.Server.Size",
	"ThreadPool.Server.SizeMax",
	"ThreadPool.Server.SizeWarn",
	"ThreadPool.Server.StackSize",
	"ThreadPerConnection",
	"ThreadPerConnection.StackSize",
	"Trace.GC",
	"Trace.Location",
	"Trace.Network",
	"Trace.Protocol",
	"Trace.Retry",
	"Trace.Slicing",
	"UDP.RcvSize",
	"UDP.SndSize",
	"UseEventLog",
	"UseSyslog",
	"Warn.AMICallback",
	"Warn.Connections",
	"Warn.Datagrams",
	"Warn.Dispatch",
	"Warn.Leaks"
    };
    
    private static final String _iceBoxProps[] =
    {
	"LoadOrder",
	"PrintServicesReady",
	"Service.*",
	"ServiceManager.AdapterId",
	"ServiceManager.Endpoints",
	"ServiceManager.PublishedEndpoints",
	"ServiceManager.Identity",
	"ServiceManager.RegisterProcess",
	"ServiceManager.ThreadPool.Size",
	"ServiceManager.ThreadPool.SizeMax",
	"ServiceManager.ThreadPool.SizeWarn",
	"ServiceManager.ThreadPool.StackSize",
	"UseSharedCommunicator.*"
    };

    private static final String _icePackProps[] =
    {
	"Node.AdapterId",
	"Node.CollocateRegistry",
	"Node.Data",
	"Node.Endpoints",
	"Node.PublishedEndpoints",
	"Node.Name",
	"Node.Output",
	"Node.PrintServersReady",
	"Node.PropertiesOverride",
	"Node.RedirectErrToOut",
	"Node.ThreadPool.Size",
	"Node.ThreadPool.SizeMax",
	"Node.ThreadPool.SizeWarn",
	"Node.ThreadPool.StackSize",
	"Node.Trace.Activator",
	"Node.Trace.Adapter",
	"Node.Trace.Server",
	"Node.WaitTime",
	"Registry.Admin.AdapterId",
	"Registry.Admin.Endpoints",
	"Registry.Admin.PublishedEndpoints",
	"Registry.Admin.ThreadPool.Size",
	"Registry.Admin.ThreadPool.SizeMax",
	"Registry.Admin.ThreadPool.SizeWarn",
	"Registry.Admin.ThreadPool.StackSize",
	"Registry.Client.AdapterId",
	"Registry.Client.Endpoints",
	"Registry.Client.PublishedEndpoints",
	"Registry.Client.ThreadPool.Size",
	"Registry.Client.ThreadPool.SizeMax",
	"Registry.Client.ThreadPool.SizeWarn",
	"Registry.Client.ThreadPool.StackSize",
	"Registry.Data",
	"Registry.DynamicRegistration",
	"Registry.Internal.AdapterId",
	"Registry.Internal.Endpoints",
	"Registry.Internal.PublishedEndpoints",
	"Registry.Internal.ThreadPool.Size",
	"Registry.Internal.ThreadPool.SizeMax",
	"Registry.Internal.ThreadPool.SizeWarn",
	"Registry.Internal.ThreadPool.StackSize",
	"Registry.Server.AdapterId",
	"Registry.Server.Endpoints",
	"Registry.Server.PublishedEndpoints",
	"Registry.Server.ThreadPool.Size",
	"Registry.Server.ThreadPool.SizeMax",
	"Registry.Server.ThreadPool.SizeWarn",
	"Registry.Server.ThreadPool.StackSize",
	"Registry.Trace.AdapterRegistry",
	"Registry.Trace.NodeRegistry",
	"Registry.Trace.ObjectRegistry",
	"Registry.Trace.ServerRegistry"
    };

    private static final String _icePatchProps[] =
    {
	"BusyTimeout",
	"Directory",
	"Dynamic",
	"Endpoints",
	"PublishedEndpoints",
	"RemoveOrphaned",
	"Thorough",
	"Trace.Files",
	"UpdatePeriod"
    };
    
    private static final String _iceSSLProps[] =
    {
	"Client.CertPath*",
	"Client.Config",
	"Client.Handshake.Retries",
	"Client.IgnoreValidPeriod",
	"Client.Overrides.CACertificate",
	"Client.Overrides.DSA.Certificate",
	"Client.Overrides.DSA.PrivateKey",
	"Client.Overrides.RSA.Certificate",
	"Client.Overrides.RSA.PrivateKey",
	"Client.Passphrase.Retries",
	"Server.CertPath*",
	"Server.Config",
	"Server.IgnoreValidPeriod",
	"Server.Overrides.CACertificate",
	"Server.Overrides.DSA.Certificate",
	"Server.Overrides.DSA.PrivateKey",
	"Server.Overrides.RSA.Certificate",
	"Server.Overrides.RSA.PrivateKey",
	"Server.Passphrase.Retries",
	"Trace.Security"
    };

    private static final String _iceStormProps[] =
    {
	"Flush.Timeout",
	"Publish.AdapterId",
	"Publish.Endpoints",
	"Publish.PublishedEndpoints",
	"Publish.RegisterProcess",
	"Publish.ThreadPool.Size",
	"Publish.ThreadPool.SizeMax",
	"Publish.ThreadPool.SizeWarn",
	"Publish.ThreadPool.StackSize",
	"TopicManager.AdapterId",
	"TopicManager.Endpoints",
	"TopicManager.PublishedEndpoints",
	"TopicManager.Proxy",
	"TopicManager.RegisterProcess",
	"TopicManager.ThreadPool.Size",
	"TopicManager.ThreadPool.SizeMax",
	"TopicManager.ThreadPool.SizeWarn",
	"TopicManager.ThreadPool.StackSize",
	"Trace.Flush",
	"Trace.Subscriber",
	"Trace.Topic",
	"Trace.TopicManager"
    };

    private static final String _glacierProps[] =
    {
	"Router.AcceptCert",
	"Router.AdapterId",
	"Router.AllowCategories",
	"Router.Client.AdapterId",
	"Router.Client.Endpoints",
	"Router.Client.PublishedEndpoints",
	"Router.Client.ForwardContext",
	"Router.Client.SleepTime",
	"Router.Client.ThreadPool.Size",
	"Router.Client.ThreadPool.SizeMax",
	"Router.Client.ThreadPool.SizeWarn",
	"Router.Client.ThreadPool.StackSize",
	"Router.Client.Throttle.Twoways",
	"Router.Endpoints",
	"Router.PublishedEndpoints",
	"Router.ThreadPool.Size",
	"Router.ThreadPool.SizeMax",
	"Router.ThreadPool.SizeWarn",
	"Router.ThreadPool.StackSize",
	"Router.Identity",
	"Router.PrintProxyOnFd",
	"Router.Server.AdapterId",
	"Router.Server.Endpoints",
	"Router.Server.PublishedEndpoints",
	"Router.Server.ForwardContext",
	"Router.Server.SleepTime",
	"Router.Server.ThreadPool.Size",
	"Router.Server.ThreadPool.SizeMax",
	"Router.Server.ThreadPool.SizeWarn",
	"Router.Server.ThreadPool.StackSize",
	"Router.Server.Throttle.Twoways",
	"Router.SessionManager",
	"Router.Trace.Client",
	"Router.Trace.RoutingTable",
	"Router.Trace.Server",
	"Router.Trace.Throttle",
	"Router.UserId",
	"Starter.AdapterId",
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
	"Starter.PublishedEndpoints",
	"Starter.PermissionsVerifier",
	"Starter.PropertiesOverride",
	"Starter.RouterPath",
	"Starter.StartupTimeout",
	"Starter.ThreadPool.Size",
	"Starter.ThreadPool.SizeMax",
	"Starter.ThreadPool.SizeWarn",
	"Starter.ThreadPool.StackSize",
	"Starter.Trace"
    };

    private static final String _glacier2Props[] =
    {
	"AddUserToAllowCategories",
	"Admin.Endpoints",
	"Admin.PublishedEndpoints",
	"AdminIdentity",
	"AllowCategories",
	"Client.AlwaysBatch",
	"Client.Endpoints",
	"Client.PublishedEndpoints",
	"Client.ForwardContext",
	"Client.SleepTime",
	"Client.Trace.Override",
	"Client.Trace.Reject",
	"Client.Trace.Request",
	"Client.Buffered",
	"CryptPasswords",
	"PermissionsVerifier",
	"RouterIdentity",
	"Server.AlwaysBatch",
	"Server.Endpoints",
	"Server.PublishedEndpoints",
	"Server.ForwardContext",
	"Server.SleepTime",
	"Server.Trace.Override",
	"Server.Trace.Request",
	"Server.Buffered",
	"SessionManager",
	"SessionTimeout",
	"Trace.RoutingTable",
	"Trace.Session"
    };

    private static final String _freezeProps[] =
    {
	"DbEnv.*",
	"Evictor.*",
	"Trace.DbEnv",
	"Trace.Evictor",
	"Trace.Map",
        "Trace.Transaction",
	"Warn.CloseInFinalize",
	"Warn.Deadlocks"
    };

    private static final class ValidProps
    {
        ValidProps()
	{
	    _validProps.put("Freeze", _freezeProps);
	    _validProps.put("Glacier", _glacierProps);
	    _validProps.put("Glacier2", _glacier2Props);
	    _validProps.put("IceBox", _iceBoxProps);
	    _validProps.put("Ice", _iceProps);
	    _validProps.put("IcePack", _icePackProps);
	    _validProps.put("IcePatch", _icePatchProps);
	    _validProps.put("IceSSL", _iceSSLProps);
	    _validProps.put("IceStorm", _iceStormProps);
	}

        public static java.util.HashMap _validProps = new java.util.HashMap();
    };

    private static final ValidProps _vp = new ValidProps();
}
