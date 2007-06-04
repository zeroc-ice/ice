// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class PropertiesI implements Properties
{
    class PropertyValue
    {
        public PropertyValue(String v, boolean u)
        {
            value = v;
            used = u;
        }

        public String value;
        public boolean used;
    }

    public synchronized String
    getProperty(String key)
    {
        String result = null;
        PropertyValue pv = (PropertyValue)_properties.get(key);
        if(pv == null)
        {
            result = System.getProperty(key);
        }
        else
        {
            pv.used = true;
            result = pv.value;
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
        String result = null;
        PropertyValue pv = (PropertyValue)_properties.get(key);
        if(pv == null)
        {
            result = System.getProperty(key);
        }
        else
        {
            pv.used = true;
            result = pv.value;
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
        String result = null;
        PropertyValue pv = (PropertyValue)_properties.get(key);
        if(pv == null)
        {
            result = System.getProperty(key);
        }
        else
        {
            pv.used = true;
            result = pv.value;
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
            if(prefix.length() == 0 || key.startsWith(prefix))
            {
                PropertyValue pv = (PropertyValue)entry.getValue();
                pv.used = true;
                result.put(key, pv.value);
            }
        }
        return result;
    }

    public void
    setProperty(String key, String value)
    {
        //
        // Check if the property is legal.
        //
        Logger logger = Ice.Util.getProcessLogger();
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
                String pattern = IceInternal.PropertyNames.validProps[i][0].pattern();
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
                    pattern = IceInternal.PropertyNames.validProps[i][j].pattern();
                    java.util.regex.Pattern pComp = java.util.regex.Pattern.compile(pattern);
                    java.util.regex.Matcher m = pComp.matcher(key);
                    found = m.matches();

                    if(found && IceInternal.PropertyNames.validProps[i][j].deprecated())
                    {
                        logger.warning("deprecated property: " + key);
                        if(IceInternal.PropertyNames.validProps[i][j].deprecatedBy() != null)
                        {
                            key = IceInternal.PropertyNames.validProps[i][j].deprecatedBy();
                        }
                    }
                }
                if(!found)
                {
                    logger.warning("unknown property: " + key);
                }
            }
        }

        synchronized(this)
        {
            //
            // Set or clear the property.
            //
            if(value != null && value.length() > 0)
            {
                PropertyValue pv = (PropertyValue)_properties.get(key);
                if(pv != null)
                {
                    pv.value = value;
                }
                else
                {
                    pv = new PropertyValue(value, false);
                }
                _properties.put(key, pv);
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
            result[i++] = "--" + entry.getKey() + "=" + ((PropertyValue)entry.getValue()).value;
        }
        assert(i == result.length);
        return result;
    }

    public String[]
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

    public String[]
    parseIceCommandLineOptions(String[] options)
    {
        String[] args = options;
        for(int i = 0; IceInternal.PropertyNames.clPropNames[i] != null; ++i)
        {
            args = parseCommandLineOptions(IceInternal.PropertyNames.clPropNames[i], args);
        }
        return args;
    }

    public void
    load(String file)
    {
        try
        {
            java.io.FileInputStream fis = new java.io.FileInputStream(file);
            java.io.InputStreamReader isr = new java.io.InputStreamReader(fis, "UTF-8");
            java.io.BufferedReader br = new java.io.BufferedReader(isr);
            parse(br);
        }
        catch(java.io.IOException ex)
        {
            FileException fe = new FileException();
            fe.path = file;
            fe.initCause(ex); // Exception chaining
            throw fe;
        }
    }

    public synchronized Properties
    _clone()
    {
        return new PropertiesI(this);
    }

    public synchronized java.util.List
    getUnusedProperties()
    {
        java.util.List unused = new java.util.ArrayList();
        java.util.Iterator p = _properties.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            PropertyValue pv = (PropertyValue)entry.getValue();
            if(!pv.used)
            {
                unused.add((String)entry.getKey());
            }
        }
        return unused;
    }

    PropertiesI(PropertiesI p)
    {
        _properties = new java.util.HashMap(p._properties);
    }

    PropertiesI()
    {
    }

    PropertiesI(StringSeqHolder args, Properties defaults)
    {
        if(defaults != null)
        {
            _properties = new java.util.HashMap(((PropertiesI)defaults)._properties);
        }
        
        boolean loadConfigFiles = false;

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
                loadConfigFiles = true;

                String[] arr = new String[args.value.length - 1];
                System.arraycopy(args.value, 0, arr, 0, i);
                if(i < args.value.length - 1)
                {
                    System.arraycopy(args.value, i + 1, arr, i, args.value.length - i - 1);
                }
                args.value = arr;
            }
        }

        if(loadConfigFiles)
        {
            loadConfig();
        }

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

        _properties.put("Ice.Config", new PropertyValue(value, true));
    }

    private java.util.HashMap _properties = new java.util.HashMap();
}
