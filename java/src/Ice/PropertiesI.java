// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

class PropertiesI implements Properties
{
    public synchronized String
    getProperty(String key)
    {
        String result = (String)_properties.get(key);
        if (result == null)
        {
            result = System.getProperty(key);
        }
        if (result == null)
        {
            result = "";
        }
        return result;
    }

    public synchronized String
    getPropertyWithDefault(String key, String value)
    {
        String result = (String)_properties.get(key);
        if (result == null)
        {
            result = System.getProperty(key);
        }
        if (result == null)
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
        if (result == null)
        {
            result = System.getProperty(key);
        }
        if (result == null)
        {
            return value;
        }

	try
	{
	    return Integer.parseInt(result);
	}
	catch (NumberFormatException ex)
	{
	    return 0;
	}
    }

    public synchronized String[]
    getProperties(String prefix)
    {
        java.util.ArrayList l = new java.util.ArrayList();
        java.util.Iterator p = _properties.entrySet().iterator();
        while (p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            String key = (String)entry.getKey();
            if (prefix.length() == 0 || key.startsWith(prefix))
            {
                l.add(key);
                l.add(entry.getValue());
            }
        }

        String[] result = new String[l.size()];
        l.toArray(result);
        return result;
    }

    public synchronized void
    setProperty(String key, String value)
    {
        _properties.put(key, value);
    }

    public synchronized String[]
    getCommandLineOptions()
    {
        String[] result = new String[_properties.size()];
        java.util.Iterator p = _properties.entrySet().iterator();
        int i = 0;
        while (p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            result[i++] = "--" + entry.getKey() + "=" + entry.getValue();
        }
        assert(i == result.length);
        return result;
    }

    public synchronized Properties
    _clone()
    {
        PropertiesI p = new PropertiesI(new String[0]);
        p._properties.putAll(_properties);
        return p;
    }

    static void
    addArgumentPrefix(String prefix)
    {
        _argumentPrefixes.add(prefix);
    }

    PropertiesI(String[] args)
    {
        String file = getConfigFile(args);

        if (file.length() > 0)
        {
            load(file);
        }

        StringSeqHolder argsH = new StringSeqHolder();
        argsH.value = args;
        parseArgs(argsH);
        setProperty("Ice.Config", file);
    }

    PropertiesI(StringSeqHolder args)
    {
        String file = getConfigFile(args.value);

        if (file.length() > 0)
        {
            load(file);
        }

        parseArgs(args);
        setProperty("Ice.Config", file);
    }

    PropertiesI(String[] args, String file)
    {
        if (file == null)
        {
            file = "";
        }

        if (file.length() > 0)
        {
            load(file);
        }

        StringSeqHolder argsH = new StringSeqHolder();
        argsH.value = args;
        parseArgs(argsH);
        setProperty("Ice.Config", file);
    }

    PropertiesI(StringSeqHolder args, String file)
    {
        if (file == null)
        {
            file = "";
        }

        if (file.length() > 0)
        {
            load(file);
        }

        parseArgs(args);
        setProperty("Ice.Config", file);
    }

    private String
    getConfigFile(String[] args)
    {
        for (int i = 0; i < args.length; i++)
        {
            if (args[i].startsWith("--Ice.Config"))
            {
                String line = args[i];
                if (line.indexOf('=') == -1)
                {
                    line += "=1";
                }
                parseLine(line.substring(2));
            }
        }

        String file = getProperty("Ice.Config");

        if (file.equals("1"))
        {
            file = "";
        }

        return file;
    }

    private void
    parseArgs(StringSeqHolder args)
    {
        int idx = 0;
        while (idx < args.value.length)
        {
            boolean match = false;
            String arg = args.value[idx];
            int beg = arg.indexOf("--");
            if (beg == 0)
            {
                String rest = arg.substring(2);
                if (rest.startsWith("Ice."))
                {
                    match = true;
                }
                else
                {
                    java.util.Iterator p = _argumentPrefixes.iterator();
                    while (p.hasNext())
                    {
                        String prefix = (String)p.next();
                        if (rest.startsWith(prefix + '.'))
                        {
                            match = true;
                            break;
                        }
                    }
                }

                if (match)
                {
                    String[] arr = new String[args.value.length - 1];
                    System.arraycopy(args.value, 0, arr, 0, idx);
                    if (idx < args.value.length - 1)
                    {
                        System.arraycopy(args.value, idx + 1, arr, idx, args.value.length - idx - 1);
                    }
                    args.value = arr;

                    if (arg.indexOf('=') == -1)
                    {
                        arg += "=1";
                    }

                    parseLine(arg.substring(2));
                }
            }

            if (!match)
            {
                idx++;
            }
        }
    }

    private void
    load(String file)
    {
        try
        {
            java.io.FileReader fr = new java.io.FileReader(file);
            java.io.BufferedReader br = new java.io.BufferedReader(fr);
            parse(br);
        }
        catch (java.io.IOException ex)
        {
            SystemException se = new SystemException();
            se.initCause(ex); // Exception chaining
            throw se;
        }
    }

    private void
    parse(java.io.BufferedReader in)
    {
        try
        {
            String line;
            while ((line = in.readLine()) != null)
            {
                parseLine(line);
            }
        }
        catch (java.io.IOException ex)
        {
            SystemException se = new SystemException();
            se.initCause(ex); // Exception chaining
            throw se;
        }
    }

    private void
    parseLine(String line)
    {
        String s = line;

        int hash = s.indexOf('#');
        if (hash == 0)
        {
            return; // ignore comment lines
        }
        else if (hash != -1)
        {
            s = s.substring(0, hash);
        }

        s = s.trim();

        final char[] arr = s.toCharArray();
        int end = -1;
        for (int i = 0; i < arr.length; i++)
        {
            if (arr[i] == ' ' || arr[i] == '\t' || arr[i] == '\r' || arr[i] == '\n' || arr[i] == '=')
            {
                end = i;
                break;
            }
        }
        if (end == -1)
        {
            return;
        }

        String key = s.substring(0, end);

        end = s.indexOf('=', end);
        if (end == -1)
        {
            return;
        }
        ++end;

        String value = "";
        if (end < s.length())
        {
            value = s.substring(end).trim();
        }

        setProperty(key, value);
    }

    private java.util.HashMap _properties = new java.util.HashMap();
    private static java.util.HashSet _argumentPrefixes = new java.util.HashSet();
}
