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
    public String
    getProperty(String key)
    {
        String result = (String)_properties.get(key);
        if (result == null)
        {
            result = System.getProperty(key);
        }
        return result;
    }

    public void
    setProperty(String key, String value)
    {
        _properties.put(key, value);
    }

    public Properties
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

        if (file == null || file.equals("1"))
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

        parseArgs(args);
        setProperty("Ice.Config", file);
    }

    private void
    parseArgs(String[] args)
    {
        int idx = 0;
        while (idx < args.length)
        {
            boolean match = false;
            String arg = args[idx];
            int beg = arg.indexOf("--");
            if (beg == 0)
            {
                int end = arg.indexOf('.');
                if (end != -1)
                {
                    String prefix = arg.substring(2, end);
                    if (prefix.equals("Ice") ||
                        _argumentPrefixes.contains(prefix))
                    {
                        match = true;
                    }

                    if (match)
                    {
                        if (arg.indexOf('=') == -1)
                        {
                            arg += "=1";
                        }

                        parseLine(arg.substring(2));
                    }
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
            if (arr[i] == ' ' || arr[i] == '\t' || arr[i] == '=')
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
        String value = "";
        if (end < s.length())
        {
            value = s.substring(end + 1).trim();
        }
        setProperty(key, value);
    }

    private java.util.HashMap _properties = new java.util.HashMap();
    private static java.util.HashSet _argumentPrefixes =
        new java.util.HashSet();
}
