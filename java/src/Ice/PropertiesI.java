// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
	if(key == null || key.length() == 0)
	{
	    return;
	}

        int dotPos = key.indexOf('.');
	if(dotPos != -1)
	{
	    String prefix = key.substring(0, dotPos);
	    for(int i = 0; IceInternal.PropertyNames.validProps[i] != null; ++i)
	    {
	        String pattern = IceInternal.PropertyNames.validProps[i][0];
		dotPos = pattern.indexOf('.');
		assert(dotPos != -1);
		String propPrefix = pattern.substring(1, dotPos - 1);
		if(!propPrefix.equals(prefix))
		{
		    continue;
		}

		boolean found = false;
		for(int j = 0; IceInternal.PropertyNames.validProps[i][j] != null && !found; ++j)
		{
		    pattern = IceInternal.PropertyNames.validProps[i][j];
		    java.util.regex.Pattern pComp = java.util.regex.Pattern.compile(pattern);
		    java.util.regex.Matcher m = pComp.matcher(key);
		    found = m.matches();
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
	if(value != null && value.length() > 0)
	{
	    _properties.put(key, value);
	}
	else
	{
	    _properties.remove(key);
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
}
