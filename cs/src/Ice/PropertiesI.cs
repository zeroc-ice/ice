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
		    string[] validProps = (string[])ValidProps._vp[prefix];
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
	
	public StringSeq getCommandLineOptions()
	{
	    lock(this)
	    {
		StringSeq result = new StringSeq();
		foreach(DictionaryEntry entry in _properties)
		{
		    result.Add("--" + entry.Key + "=" + entry.Value);
		}
		return result;
	    }
	}

	public StringSeq parseCommandLineOptions(string pfx, StringSeq options)
	{
	    lock(this)
	    {
		string prefix = pfx;
		if(pfx.Length > 0 && pfx[pfx.Length - 1] != '.')
		{
		    pfx += '.';
		}
		pfx = "--" + pfx;
		
		StringSeq result = new StringSeq();
		for(int i = 0; i < options.Count; i++)
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
		return result;
	    }
	}
	
	public StringSeq parseIceCommandLineOptions(StringSeq options)
	{
	    lock(this)
	    {
		StringSeq args = parseCommandLineOptions("Ice", options);
		args = parseCommandLineOptions("Freeze", args);
		args = parseCommandLineOptions("Glacier", args);
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
	
	public Properties clone()
	{
	    lock(this)
	    {
		return new PropertiesI(this);
	    }
	}
	
	internal PropertiesI(PropertiesI p)
	{
	    _properties = new PropertyDict(p._properties);
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
	    
	    StringSeq argSeq = new StringSeq(args);
	    parseIceCommandLineOptions(argSeq);
	    args = argSeq.ToArray();    
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
		return ; // ignore comment lines
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
		return ;
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
	    "Daemon",
	    "DaemonNoChdir",
	    "DaemonNoClose",
	    "Default.Host",
	    "Default.Locator",
	    "Default.Protocol",
	    "Default.Router",
	    "GC.Interval",			// Not used by C# but left here to suppress warnings.
	    "Logger.Timestamp",
	    "MessageSizeMax",
	    "MonitorConnections",
	    "Nohup",
	    "NullHandleAbort",
	    "Override.Compress",
	    "Override.ConnectTimeout",
	    "Override.Timeout",
	    "Plugin.*",
	    "PrintAdapterReady",
	    "PrintProcessId",
	    "ProgramName",
	    "RetryIntervals",
	    "ServerId",
	    "ServerIdleTime",
	    "ThreadPool.Client.Size",
	    "ThreadPool.Client.SizeMax",
	    "ThreadPool.Client.SizeWarn",
	    "ThreadPool.Server.Size",
	    "ThreadPool.Server.SizeMax",
	    "ThreadPool.Server.SizeWarn",
	    "Trace.GC",			// Not used by C# but left here to suppress warnings.
	    "Trace.Network",
	    "Trace.Protocol",
	    "Trace.Retry",
	    "Trace.Slicing",
	    "Trace.Location",
	    "UDP.RcvSize",
	    "UDP.SndSize",
	    "UseSyslog",
	    "Warn.AMICallback",
	    "Warn.Connections",
	    "Warn.Datagrams",
	    "Warn.Dispatch",
	    "Warn.Leaks"
	};
	
	private static readonly string[] _iceBoxProps = new string[]
	{
	    "DBEnvName.*",
	    "LoadOrder",
	    "PrintServicesReady",
	    "Service.*",
	    "ServiceManager.AdapterId",
	    "ServiceManager.Endpoints",
	    "ServiceManager.Identity",
	    "ServiceManager.RegisterProcess",
	    "UseSharedCommunicator.*"
	};
	
	private static readonly string[] _icePackProps = new string[]
	{
	    "Node.AdapterId",
	    "Node.CollocateRegistry",
	    "Node.Data",
	    "Node.Endpoints",
	    "Node.Name",
	    "Node.PrintServersReady",
	    "Node.PropertiesOverride",
	    "Node.ThreadPool.Size",
	    "Node.ThreadPool.SizeMax",
	    "Node.ThreadPool.SizeWarn",
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
	
	private static readonly string[] _icePatchProps = new string[]
	{
	    "BusyTimeout",
	    "RemoveOrphaned",
	    "Thorough",
	    "Trace.Files",
	    "UpdatePeriod",
	    "Directory"
	};
	
	private static readonly string[] _iceSSLProps = new string[]
	{
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
	
	private static readonly string[] _iceStormProps = new string[]
	{
	    "Flush.Timeout",
	    "Publish.Endpoints",
	    "Publish.AdapterId",
	    "TopicManager.AdapterId",
	    "TopicManager.Endpoints",
	    "TopicManager.Proxy",
	    "Trace.Flush",
	    "Trace.Subscriber",
	    "Trace.Topic",
	    "Trace.TopicManager"
	};
	    
	private static readonly string[] _glacierProps = new string[]
	{
	    "Router.AcceptCert",
	    "Router.AllowCategories",
	    "Router.Client.BatchSleepTime",
	    "Router.Client.Endpoints",
	    "Router.Client.ThreadPool.Size",
	    "Router.Client.ThreadPool.SizeMax",
	    "Router.Client.ThreadPool.SizeWarn",
	    "Router.Client.ForwardContext",
	    "Router.Endpoints",
	    "Router.ThreadPool.Size",
	    "Router.ThreadPool.SizeMax",
	    "Router.ThreadPool.SizeWarn",
	    "Router.Identity",
	    "Router.PrintProxyOnFd",
	    "Router.Server.BatchSleepTime",
	    "Router.Server.Endpoints",
	    "Router.Server.ThreadPool.Size",
	    "Router.Server.ThreadPool.SizeMax",
	    "Router.Server.ThreadPool.SizeWarn",
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
	    "Starter.PermissionsVerifier",
	    "Starter.PropertiesOverride",
	    "Starter.RouterPath",
	    "Starter.StartupTimeout",
	    "Starter.Trace"
	};
	    
	private static readonly string[] _freezeProps = new string[]
	{
	    "Warn.Deadlocks",
	    "Warn.CloseInFinalize",
	    "Trace.Map",
	    "Trace.Evictor",
	    "Trace.DbEnv",
	    "Evictor.*",
	    "DbEnv.*"
	};
	    
	private sealed class ValidProps
	{
	    static ValidProps()
	    {
		_vp = new Hashtable();
		_vp["Freeze"] = Ice.PropertiesI._freezeProps;
		_vp["Glacier"] = Ice.PropertiesI._glacierProps;
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
