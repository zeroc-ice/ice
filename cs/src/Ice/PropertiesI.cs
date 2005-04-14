// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections;
using System.Diagnostics;
using System.Text.RegularExpressions;

namespace Ice
{

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
	    if(key == null || key.Length == 0)
	    {
	        return;
	    }

	    // Check if the property is legal. (We write to Console.Error instead of using
	    // a logger because no logger may be established at the time the property
	    // is parsed.)
	    //
	    int dotPos = key.IndexOf('.');
	    if(dotPos != -1)
	    {
		string prefix = key.Substring(0, dotPos);
		for(int i = 0; IceInternal.PropertyNames.validProps[i] != null; ++i)
		{
		    string pattern = IceInternal.PropertyNames.validProps[i][0];
		    dotPos = pattern.IndexOf('.');
		    Debug.Assert(dotPos != -1);
		    string propPrefix = pattern.Substring(1, dotPos - 2);
		    if(!propPrefix.Equals(prefix))
		    {
			continue;
		    }

		    bool found = false;
		    for(int j = 0; IceInternal.PropertyNames.validProps[i][j] != null && !found; ++j)
		    {
			Regex r = new Regex(IceInternal.PropertyNames.validProps[i][j]);
			Match m = r.Match(key);
			found = m.Success;
		    }
		    if(!found)
		    {
			System.Console.Error.WriteLine("warning: unknown property: " + key);
		    }
		}
	    }

	    lock(this)
	    {
		//
		//
		// Set or clear the property.
		//
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
    }

}
