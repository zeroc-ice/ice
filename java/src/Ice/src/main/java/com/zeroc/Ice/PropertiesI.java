// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

public final class PropertiesI implements Properties
{
    static class PropertyValue
    {
        public PropertyValue(PropertyValue v)
        {
            value = v.value;
            used = v.used;
        }

        public PropertyValue(String v, boolean u)
        {
            value = v;
            used = u;
        }

        public String value;
        public boolean used;
    }

    @Override
    public synchronized String
    getProperty(String key)
    {
        PropertyValue pv = _properties.get(key);
        if(pv != null)
        {
            pv.used = true;
            return pv.value;
        }
        else
        {
            return "";
        }
    }

    @Override
    public synchronized String
    getPropertyWithDefault(String key, String value)
    {
        PropertyValue pv = _properties.get(key);
        if(pv != null)
        {
            pv.used = true;
            return pv.value;
        }
        else
        {
            return value;
        }
    }

    @Override
    public int
    getPropertyAsInt(String key)
    {
        return getPropertyAsIntWithDefault(key, 0);
    }

    @Override
    public synchronized int
    getPropertyAsIntWithDefault(String key, int value)
    {
        PropertyValue pv = _properties.get(key);
        if(pv != null)
        {
            pv.used = true;

            try
            {
                return Integer.parseInt(pv.value);
            }
            catch(NumberFormatException ex)
            {
                Util.getProcessLogger().warning("numeric property " + key +
                                                " set to non-numeric value, defaulting to " + value);
            }
        }

        return value;
    }

    @Override
    public String[]
    getPropertyAsList(String key)
    {
        return getPropertyAsListWithDefault(key, null);
    }

    @Override
    public synchronized String[]
    getPropertyAsListWithDefault(String key, String[] value)
    {
        if(value == null)
        {
            value = new String[0];
        }

        PropertyValue pv = _properties.get(key);
        if(pv != null)
        {
            pv.used = true;

            String[] result = com.zeroc.IceUtilInternal.StringUtil.splitString(pv.value, ", \t\r\n");
            if(result == null)
            {
                Util.getProcessLogger().warning("mismatched quotes in property " + key +
                                                "'s value, returning default value");
                return value;
            }
            if(result.length == 0)
            {
                result = value;
            }
            return result;
        }
        else
        {
            return value;
        }
    }

    @Override
    public synchronized java.util.Map<String, String>
    getPropertiesForPrefix(String prefix)
    {
        java.util.HashMap<String, String> result = new java.util.HashMap<>();
        for(java.util.Map.Entry<String, PropertyValue> p : _properties.entrySet())
        {
            String key = p.getKey();
            if(prefix.length() == 0 || key.startsWith(prefix))
            {
                PropertyValue pv = p.getValue();
                pv.used = true;
                result.put(key, pv.value);
            }
        }
        return result;
    }

    @Override
    public void
    setProperty(String key, String value)
    {
        //
        // Trim whitespace
        //
        if(key != null)
        {
            key = key.trim();
        }

        //
        // Check if the property is legal.
        //
        Logger logger = Util.getProcessLogger();
        if(key == null || key.length() == 0)
        {
            throw new InitializationException("Attempt to set property with empty key");
        }

        int dotPos = key.indexOf('.');
        if(dotPos != -1)
        {
            String prefix = key.substring(0, dotPos);
            for(int i = 0; com.zeroc.IceInternal.PropertyNames.validProps[i] != null; ++i)
            {
                String pattern = com.zeroc.IceInternal.PropertyNames.validProps[i][0].pattern();
                dotPos = pattern.indexOf('.');
                //
                // Each top level prefix describes a non-empty namespace. Having a string without a
                // prefix followed by a dot is an error.
                //
                assert(dotPos != -1);
                String propPrefix = pattern.substring(0, dotPos - 1);
                boolean mismatchCase = false;
                String otherKey = "";
                if(!propPrefix.toUpperCase().equals(prefix.toUpperCase()))
                {
                    continue;
                }

                boolean found = false;
                for(int j = 0; com.zeroc.IceInternal.PropertyNames.validProps[i][j] != null && !found; ++j)
                {
                    pattern = com.zeroc.IceInternal.PropertyNames.validProps[i][j].pattern();
                    java.util.regex.Pattern pComp = java.util.regex.Pattern.compile(pattern);
                    java.util.regex.Matcher m = pComp.matcher(key);
                    found = m.matches();

                    if(found && com.zeroc.IceInternal.PropertyNames.validProps[i][j].deprecated())
                    {
                        logger.warning("deprecated property: " + key);
                        if(com.zeroc.IceInternal.PropertyNames.validProps[i][j].deprecatedBy() != null)
                        {
                            key = com.zeroc.IceInternal.PropertyNames.validProps[i][j].deprecatedBy();
                        }
                    }

                    if(!found)
                    {
                        pComp = java.util.regex.Pattern.compile(pattern.toUpperCase());
                        m = pComp.matcher(key.toUpperCase());
                        if(m.matches())
                        {
                            found = true;
                            mismatchCase = true;
                            otherKey = pattern.replaceAll("\\\\", "");
                            break;
                        }
                    }
                }
                if(!found)
                {
                    logger.warning("unknown property: " + key);
                }
                else if(mismatchCase)
                {
                    logger.warning("unknown property: `" + key + "'; did you mean `" + otherKey + "'");
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
                PropertyValue pv = _properties.get(key);
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

    @Override
    public synchronized String[]
    getCommandLineOptions()
    {
        String[] result = new String[_properties.size()];
        int i = 0;
        for(java.util.Map.Entry<String, PropertyValue> p : _properties.entrySet())
        {
            result[i++] = "--" + p.getKey() + "=" + p.getValue().value;
        }
        assert(i == result.length);
        return result;
    }

    @Override
    public String[]
    parseCommandLineOptions(String pfx, String[] options)
    {
        if(pfx.length() > 0 && pfx.charAt(pfx.length() - 1) != '.')
        {
            pfx += '.';
        }
        pfx = "--" + pfx;

        java.util.ArrayList<String> result = new java.util.ArrayList<>();
        for(String opt : options)
        {
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
        return result.toArray(new String[0]);
    }

    @Override
    public String[]
    parseIceCommandLineOptions(String[] options)
    {
        String[] args = options;
        for(int i = 0; com.zeroc.IceInternal.PropertyNames.clPropNames[i] != null; ++i)
        {
            args = parseCommandLineOptions(com.zeroc.IceInternal.PropertyNames.clPropNames[i], args);
        }
        return args;
    }

    @Override
    public void
    load(String file)
    {
        if(System.getProperty("os.name").startsWith("Windows") && file.startsWith("HKLM\\"))
        {
            String regQuery = "reg query " + file;
            try
            {
                java.lang.Process process = Runtime.getRuntime().exec(regQuery);
                process.waitFor();
                if(process.exitValue() != 0)
                {
                    InitializationException ie = new InitializationException();
                    ie.reason = "Could not read Windows registry key `" + file + "'";
                    throw ie;
                }

                java.io.InputStream is = process.getInputStream();
                java.io.StringWriter sw = new java.io.StringWriter();
                int c;
                while((c = is.read()) != -1)
                {
                    sw.write(c);
                }
                String[] result = sw.toString().split("\n");

                for(String line : result)
                {
                    int pos = line.indexOf("REG_SZ");
                    if(pos != -1)
                    {
                        setProperty(line.substring(0, pos).trim(), line.substring(pos + 6, line.length()).trim());
                        continue;
                    }

                    pos = line.indexOf("REG_EXPAND_SZ");
                    if(pos != -1)
                    {
                        String name = line.substring(0, pos).trim();
                        line = line.substring(pos + 13, line.length()).trim();
                        while(true)
                        {
                            int start = line.indexOf("%", 0);
                            int end = line.indexOf("%", start + 1);

                            //
                            // If there isn't more %var% break the loop
                            //
                            if(start == -1 || end == -1)
                            {
                                break;
                            }

                            String envKey = line.substring(start + 1, end);
                            String envValue = System.getenv(envKey);
                            if(envValue == null)
                            {
                                envValue = "";
                            }

                            envKey = "%" + envKey + "%";
                            do
                            {
                                line = line.replace(envKey , envValue);
                            }
                            while(line.indexOf(envKey) != -1);
                        }
                        setProperty(name, line);
                        continue;
                    }
                }
            }
            catch(LocalException ex)
            {
                throw ex;
            }
            catch(java.lang.Exception ex)
            {
                throw new InitializationException("Could not read Windows registry key `" + file + "'", ex);
            }
        }
        else
        {
            java.io.PushbackInputStream is = null;
            try
            {
                java.io.InputStream f = com.zeroc.IceInternal.Util.openResource(getClass().getClassLoader(), file);
                if(f == null)
                {
                    FileException fe = new FileException();
                    fe.path = file;
                    throw fe;
                }
                //
                // Skip UTF-8 BOM if present.
                //
                byte[] bom = new byte[3];
                is = new java.io.PushbackInputStream(f, bom.length);
                int read = is.read(bom, 0, bom.length);
                if(read < 3 || bom[0] != (byte)0xEF || bom[1] != (byte)0xBB || bom[2] !=  (byte)0xBF)
                {
                    if(read > 0)
                    {
                        is.unread(bom, 0, read);
                    }
                }

                java.io.InputStreamReader isr = new java.io.InputStreamReader(is, "UTF-8");
                java.io.BufferedReader br = new java.io.BufferedReader(isr);
                parse(br);
            }
            catch(java.io.IOException ex)
            {
                throw new FileException(0, file, ex);
            }
            finally
            {
                if(is != null)
                {
                    try
                    {
                        is.close();
                    }
                    catch(Throwable ex)
                    {
                        // Ignore.
                    }
                }
            }
        }
    }

    @Override
    public synchronized Properties
    _clone()
    {
        return new PropertiesI(this);
    }

    public synchronized java.util.List<String>
    getUnusedProperties()
    {
        java.util.List<String> unused = new java.util.ArrayList<>();
        for(java.util.Map.Entry<String, PropertyValue> p : _properties.entrySet())
        {
            PropertyValue pv = p.getValue();
            if(!pv.used)
            {
                unused.add(p.getKey());
            }
        }
        return unused;
    }

    PropertiesI(PropertiesI props)
    {
        //
        // NOTE: we can't just do a shallow copy of the map as the map values
        // would otherwise be shared between the two PropertiesI object.
        //
        //_properties = new java.util.HashMap<String, PropertyValue>(props._properties);
        for(java.util.Map.Entry<String, PropertyValue> p : props._properties.entrySet())
        {
            _properties.put(p.getKey(), new PropertyValue(p.getValue()));
        }
    }

    PropertiesI()
    {
    }

    String[] init(String[] args, Properties defaults)
    {
        if(defaults != null)
        {
            //
            // NOTE: we can't just do a shallow copy of the map as the map values
            // would otherwise be shared between the two PropertiesI object.
            //
            //_properties = new java.util.HashMap<>(((PropertiesI)defaults)._properties);
            for(java.util.Map.Entry<String, PropertyValue> p : (((PropertiesI)defaults)._properties).entrySet())
            {
                _properties.put(p.getKey(), new PropertyValue(p.getValue()));
            }
        }

        boolean loadConfigFiles = false;

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
                loadConfigFiles = true;

                String[] arr = new String[args.length - 1];
                System.arraycopy(args, 0, arr, 0, i);
                if(i < args.length - 1)
                {
                    System.arraycopy(args, i + 1, arr, i, args.length - i - 1);
                }
                args = arr;
            }
        }

        if(!loadConfigFiles)
        {
            //
            // If Ice.Config is not set, load from ICE_CONFIG (if set)
            //
            loadConfigFiles = !_properties.containsKey("Ice.Config");
        }

        if(loadConfigFiles)
        {
            loadConfig();
        }

        return parseIceCommandLineOptions(args);
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
            throw new SyscallException(ex);
        }
    }

    private static final int ParseStateKey = 0;
    private static final int ParseStateValue = 1;

    private void
    parseLine(String line)
    {
        String key = "";
        String value = "";

        int state = ParseStateKey;

        String whitespace = "";
        String escapedspace = "";
        boolean finished = false;
        for(int i = 0; i < line.length(); ++i)
        {
            char c = line.charAt(i);
            switch(state)
            {
              case ParseStateKey:
              {
                  switch(c)
                  {
                    case '\\':
                      if(i < line.length() - 1)
                      {
                          c = line.charAt(++i);
                          switch(c)
                          {
                            case '\\':
                            case '#':
                            case '=':
                              key += whitespace;
                              whitespace = "";
                              key += c;
                              break;

                            case ' ':
                              if(key.length() != 0)
                              {
                                  whitespace += c;
                              }
                              break;

                            default:
                              key += whitespace;
                              whitespace = "";
                              key += '\\';
                              key += c;
                              break;
                          }
                      }
                      else
                      {
                          key += whitespace;
                          key += c;
                      }
                      break;

                    case ' ':
                    case '\t':
                    case '\r':
                    case '\n':
                        if(key.length() != 0)
                        {
                            whitespace += c;
                        }
                        break;

                    case '=':
                        whitespace = "";
                        state = ParseStateValue;
                        break;

                    case '#':
                        finished = true;
                        break;

                    default:
                        key += whitespace;
                        whitespace = "";
                        key += c;
                        break;
                  }
                  break;
              }

              case ParseStateValue:
              {
                  switch(c)
                  {
                    case '\\':
                      if(i < line.length() - 1)
                      {
                          c = line.charAt(++i);
                          switch(c)
                          {
                            case '\\':
                            case '#':
                            case '=':
                              value += value.length() == 0 ? escapedspace : whitespace;
                              whitespace = "";
                              escapedspace = "";
                              value += c;
                              break;

                            case ' ':
                              whitespace += c;
                              escapedspace += c;
                              break;

                            default:
                              value += value.length() == 0 ? escapedspace : whitespace;
                              whitespace = "";
                              escapedspace = "";
                              value += '\\';
                              value += c;
                              break;
                          }
                      }
                      else
                      {
                          value += value.length() == 0 ? escapedspace : whitespace;
                          value += c;
                      }
                      break;

                    case ' ':
                    case '\t':
                    case '\r':
                    case '\n':
                        if(value.length() != 0)
                        {
                            whitespace += c;
                        }
                        break;

                    case '#':
                        finished = true;
                        break;

                    default:
                        value += value.length() == 0 ? escapedspace : whitespace;
                        whitespace = "";
                        escapedspace = "";
                        value += c;
                        break;
                  }
                  break;
              }
            }
            if(finished)
            {
                break;
            }
        }
        value += escapedspace;

        if((state == ParseStateKey && key.length() != 0) || (state == ParseStateValue && key.length() == 0))
        {
            Util.getProcessLogger().warning("invalid config file entry: \"" + line + "\"");
            return;
        }
        else if(key.length() == 0)
        {
            return;
        }

        setProperty(key, value);
    }

    private void
    loadConfig()
    {
        String value = getProperty("Ice.Config");

        if(value.length() == 0 || value.equals("1"))
        {
            try
            {
                value = System.getenv("ICE_CONFIG");
                if(value == null)
                {
                    value = "";
                }
            }
            catch(java.lang.SecurityException ex)
            {
                value = "";
            }
        }

        if(value.length() > 0)
        {
            for(String file : value.split(","))
            {
                load(file.trim());
            }

            _properties.put("Ice.Config", new PropertyValue(value, true));
        }
    }

    private java.util.HashMap<String, PropertyValue> _properties = new java.util.HashMap<>();
}
