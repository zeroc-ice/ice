// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
	
    using System.Collections;

    sealed class PropertiesI : LocalObjectImpl, Properties
    {
	public string getProperty(string key)
	{
	    lock(this)
	    {
		string result = _properties[key];
		if(result == null)
		{
		    result = "";
		}
		return result;
	    }
	}
	
	public string getPropertyWithDefault(string key, string val)
	{
	    lock(this)
	    {
		string result = _properties[key];
		if(result == null)
		{
		    result = val;
		}
		return result;
	    }
	}
	
	public int getPropertyAsInt(string key)
	{
	    return getPropertyAsIntWithDefault(key, 0);
	}
	
	public int getPropertyAsIntWithDefault(string key, int val)
	{
	    lock(this)
	    {
		string result = _properties[key];
		if(result == null)
		{
		    return val;
		}
		try
		{
		    return System.Int32.Parse(result);
		}
		catch(System.FormatException)
		{
		    return val;
		}
	    }
	}
	
	public Ice.PropertyDict getPropertiesForPrefix(string prefix)
	{
	    lock(this)
	    {
		Ice.PropertyDict result = new Ice.PropertyDict();

		foreach(string s in _properties.Keys)
		{
		    if(prefix.Length == 0 || s.StartsWith(prefix))
		    {
			result[s] = _properties[s];
		    }
		}
		return result;
	    }
	}
	
	public void setProperty(string key, string val)
	{
	    lock(this)
	    {
		//
		// Check if the property is legal. (We write to Console.Error instead of using
		// a logger because no logger may be established at the time the property
		// is parsed.)
		//
		int dotPos = key.IndexOf('.');
		if(dotPos != -1)
		{
		    string prefix = key.Substring(0, dotPos);
		    string[] validProps = (string[])(ValidProps._vp[prefix]);
		    if(validProps != null)
		    {
			string suffix = key.Substring(dotPos + 1, key.Length - dotPos - 1);
			bool found = false;
			for(int j = 0; j < validProps.Length && !found; ++j)
			{
			    int starPos = validProps[j].IndexOf('*');
			    if(starPos == -1)
			    {
				found = validProps[j].Equals(suffix);
			    }
			    else
			    {
				int max = System.Math.Min(suffix.Length, starPos);
				string s1 = suffix.Substring(0, max);
				string s2 = validProps[j].Substring(0, max);
				found = s1.Equals(s2);
			    }
			}
			if(!found)
			{
			    System.Console.Error.WriteLine("warning: unknown property: " + key);
			}
		    }
		}
		
		//
		// Set or clear the property.
		//
		if(key != null && key.Length > 0)
		{
		    if(val != null && val.Length > 0)
		    {
			_properties[key] = val;
		    }
		    else
		    {
			_properties.Remove(key);
		    }
		}
	    }
	}
	
	public string[] getCommandLineOptions()
	{
	    lock(this)
	    {
		string[] result = new string[_properties.Count];
                int i = 0;
		foreach(DictionaryEntry entry in _properties)
		{
		    result[i++] = "--" + entry.Key + "=" + entry.Value;
		}
		return result;
	    }
	}

	public string[] parseCommandLineOptions(string pfx, string[] options)
	{
	    lock(this)
	    {
		string prefix = pfx;
		if(pfx.Length > 0 && pfx[pfx.Length - 1] != '.')
		{
		    pfx += '.';
		}
		pfx = "--" + pfx;
		
		ArrayList result = new ArrayList();
		for(int i = 0; i < options.Length; i++)
		{
		    string opt = options[i];
		    if(opt.StartsWith(pfx))
		    {
			if(opt.IndexOf('=') == -1)
			{
			    opt += "=1";
			}
			
			parseLine(opt.Substring(2));
		    }
		    else
		    {
			result.Add(opt);
		    }
		}
                string[] arr = new string[result.Count];
		if(arr.Length != 0)
		{
		    result.CopyTo(arr);
		}
		return arr;
	    }
	}
	
	public string[] parseIceCommandLineOptions(string[] options)
	{
	    lock(this)
	    {
		string[] args = parseCommandLineOptions("Ice", options);
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
	}
	
	public void load(string file)
	{
	    lock(this)
	    {
		try
		{
		    System.IO.StreamReader sr = new System.IO.StreamReader(file);
		    parse(sr);
		}
		catch(System.IO.IOException ex)
		{
		    SyscallException se = new SyscallException(ex);
		    throw se;
		}
	    }
	}
	
	public Properties _Ice_clone()
	{
	    lock(this)
	    {
		return new PropertiesI(this);
	    }
	}
	
	internal PropertiesI(PropertiesI p)
	{
	    _properties = (PropertyDict)p._properties.Clone();
	}

	internal PropertiesI()
	{
	    _properties = new PropertyDict();
	    loadConfig();
	}
	
	internal PropertiesI(ref string[] args)
	{
	    _properties = new PropertyDict();
	    setProperty("Ice.ProgramName", System.AppDomain.CurrentDomain.FriendlyName);
	    for(int i = 0; i < args.Length; i++)
	    {
		if(args[i].StartsWith("--Ice.Config"))
		{
		    string line = args[i];
		    if(line.IndexOf('=') == -1)
		    {
			line += "=1";
		    }
		    parseLine(line.Substring(2));
		    string[] arr = new string[args.Length - 1];
		    System.Array.Copy(args, 0, arr, 0, i);
		    if(i < args.Length - 1)
		    {
			System.Array.Copy(args, i + 1, arr, i, args.Length - i - 1);
		    }
		    args = arr;
		}
	    }
	    
	    loadConfig();
	    
	    args = parseIceCommandLineOptions(args); 
	}
	
	private void parse(System.IO.StreamReader input)
	{
	    try
	    {
		string line;
		while((line = input.ReadLine()) != null)
		{
		    parseLine(line);
		}
	    }
	    catch(System.IO.IOException ex)
	    {
		SyscallException se = new SyscallException(ex);
		throw se;
	    }
	}
	
	private void parseLine(string line)
	{
	    string s = line;
	    
	    int hash = s.IndexOf('#');
	    if(hash == 0)
	    {
		return; // ignore comment lines
	    }
	    else if(hash != - 1)
	    {
		s = s.Substring(0, (hash) - (0));
	    }
	    
	    s = s.Trim();
	    
	    char[] arr = s.ToCharArray();
	    int end = -1;
	    for(int i = 0; i < arr.Length; i++)
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
	    
	    string key = s.Substring(0, end);
	    
	    end = s.IndexOf('=', end);
	    if(end == -1)
	    {
		return;
	    }
	    ++end;
	    
	    string val = "";
	    if(end < s.Length)
	    {
		val = s.Substring(end).Trim();
	    }
	    
	    setProperty(key, val);
	}
	
	private void loadConfig()
	{
	    string val = getProperty("Ice.Config");
	    
	    if(val.Length == 0 || val.Equals("1"))
	    {
		string s = System.Environment.GetEnvironmentVariable("ICE_CONFIG");
		if(s != null && s.Length != 0)
		{
		    val = s;
		}
	    }
	    
	    if(val.Length > 0)
	    {
		char[] separator = { ',' };
		string[] files = val.Split(separator);
		for(int i = 0; i < files.Length; i++)
		{
		    load(files[i]);
		}
	    }
	    
	    setProperty("Ice.Config", val);
	}
	
	private Ice.PropertyDict _properties;
	
	//
	// Valid properties for each application.
	// A '*' character is a wildcard. If used, it must appear at the end of the string.
	// Examples: "Ice.Foo.*" allows all properties with that prefix, such as "Ice.Foo.Bar".
	//           "Ice.Foo*" allows properties such as "Ice.Foo.Bar" and "Ice.FooBar".
	//
	private static readonly string[] _iceProps = new string[]
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
	
	private static readonly string[] _iceBoxProps = new string[]
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
	
	private static readonly string[] _icePackProps = new string[]
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
	    "Node.RedirectErrToOutput",
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
	
	private static readonly string[] _icePatchProps = new string[]
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
	
	private static readonly string[] _iceSSLProps = new string[]
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
	
	private static readonly string[] _iceStormProps = new string[]
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
	    
	private static readonly string[] _glacierProps = new string[]
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
	    
	private static readonly string[] _glacier2Props = new string[]
	{
	    "Admin.Endpoints",
	    "Admin.PublishedEndpoints",
	    "AdminIdentity",
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
	    
	private static readonly string[] _freezeProps = new string[]
	{
	    "DbEnv.*",
	    "Evictor.*",
	    "Trace.Evictor",
	    "Trace.Map",
	    "Trace.DbEnv",
	    "Warn.CloseInFinalize",
	    "Warn.Deadlocks"
	};
	    
	private sealed class ValidProps
	{
	    static ValidProps()
	    {
		_vp = new Hashtable();
		_vp["Freeze"] = Ice.PropertiesI._freezeProps;
		_vp["Glacier"] = Ice.PropertiesI._glacierProps;
		_vp["Glacier2"] = Ice.PropertiesI._glacier2Props;
		_vp["IceBox"] = Ice.PropertiesI._iceBoxProps;
		_vp["Ice"] = Ice.PropertiesI._iceProps;
		_vp["IcePack"] = Ice.PropertiesI._icePackProps;
		_vp["IcePatch"] = Ice.PropertiesI._icePatchProps;
		_vp["IceSSL"] = Ice.PropertiesI._iceSSLProps;
		_vp["IceStorm"] = Ice.PropertiesI._iceStormProps;
	    }

	    public static Hashtable _vp;
	}
    }

}
