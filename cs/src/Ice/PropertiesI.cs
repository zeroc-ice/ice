// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    sealed class PropertiesI :  Properties
    {
        class PropertyValue
        {
            public PropertyValue(string v, bool u)
            {
                val = v;
                used = u;
            }

            public string val;
            public bool used;
        }

        public string getProperty(string key)
        {
            lock(this)
            {
                string result = "";
                PropertyValue pv = (PropertyValue)_properties[key];
                if(pv != null)
                {
                    pv.used = true;
                    result = pv.val;
                }
                return result;
            }
        }
        
        public string getPropertyWithDefault(string key, string val)
        {
            lock(this)
            {
                string result = val;
                PropertyValue pv = (PropertyValue)_properties[key];
                if(pv != null)
                {
                    pv.used = true;
                    result = pv.val;
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
                PropertyValue pv = (PropertyValue)_properties[key];
                if(pv == null)
                {
                    return val;
                }
                else
                {
                    pv.used = true;
                    try
                    {
                        return System.Int32.Parse(pv.val);
                    }
                    catch(System.FormatException)
                    {
                        return val;
                    }
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
                        PropertyValue pv = (PropertyValue)_properties[s];
                        pv.used = true;
                        result[s] = pv.val;
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

            //
            // Check if the property is legal.
            //
            Logger logger = Ice.Util.getProcessLogger();
            int dotPos = key.IndexOf('.');
            if(dotPos != -1)
            {
                string prefix = key.Substring(0, dotPos);
                for(int i = 0; IceInternal.PropertyNames.validProps[i] != null; ++i)
                {
                    string pattern = IceInternal.PropertyNames.validProps[i][0].pattern();
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
                        Regex r = new Regex(IceInternal.PropertyNames.validProps[i][j].pattern());
                        Match m = r.Match(key);
                        found = m.Success;
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

            lock(this)
            {
                //
                //
                // Set or clear the property.
                //
                if(val != null && val.Length > 0)
                {
                    PropertyValue pv = (PropertyValue)_properties[key];
                    if(pv != null)
                    {
                        pv.val = val;
                    }
                    else
                    {
                        pv = new PropertyValue(val, false);
                    }
                    _properties[key] = pv;
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
                    result[i++] = "--" + entry.Key + "=" + ((PropertyValue)entry.Value).val;
                }
                return result;
            }
        }

        public string[] parseCommandLineOptions(string pfx, string[] options)
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
        
        public string[] parseIceCommandLineOptions(string[] options)
        {
            string[] args = options;
            for(int i = 0; IceInternal.PropertyNames.clPropNames[i] != null; ++i)
            {
                args = parseCommandLineOptions(IceInternal.PropertyNames.clPropNames[i], args);
            }
            return args;
        }
        
        public void load(string file)
        {
            try
            {
                using(System.IO.StreamReader sr = new System.IO.StreamReader(file))
                {
                    parse(sr);
                }
            }
            catch(System.IO.IOException ex)
            {
                Ice.FileException fe = new Ice.FileException(ex);
                fe.path = file;
                throw fe;
            }
        }
        
        public Properties ice_clone_()
        {
            lock(this)
            {
                return new PropertiesI(this);
            }
        }

        public ArrayList getUnusedProperties()
        {
            lock(this)
            {
                ArrayList unused = new ArrayList();
                foreach(DictionaryEntry entry in _properties)
                {
                    if(!((PropertyValue)entry.Value).used)
                    {
                        unused.Add(entry.Key);
                    }
                }
                return unused;
            }
        }
        
        internal PropertiesI(PropertiesI p)
        {
            _properties = (Hashtable)p._properties.Clone();
        }

        internal PropertiesI()
        {
            _properties = new Hashtable();
        }
        
        internal PropertiesI(ref string[] args, Properties defaults)
        {
            if(defaults == null)
            {
                _properties = new Hashtable();
            }
            else
            {
                _properties = ((PropertiesI)defaults)._properties;
            }
            
            PropertyValue pv = (PropertyValue)_properties["Ice.ProgramName"];
            if(pv == null)
            {
                _properties["Ice.ProgramName"] = new PropertyValue(System.AppDomain.CurrentDomain.FriendlyName, true);
            }
            else
            {
                pv.used = true;
            }

            bool loadConfigFiles = false;

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
                    loadConfigFiles = true;

                    string[] arr = new string[args.Length - 1];
                    System.Array.Copy(args, 0, arr, 0, i);
                    if(i < args.Length - 1)
                    {
                        System.Array.Copy(args, i + 1, arr, i, args.Length - i - 1);
                    }
                    args = arr;
                }
            }

            if(!loadConfigFiles)
            {
                //
                // If Ice.Config is not set, load from ICE_CONFIG (if set)
                //
                loadConfigFiles = (_properties["Ice.Config"] == null);
            }
            
            if(loadConfigFiles)
            {
                loadConfig();
            }
            
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
            
            _properties["Ice.Config"] = new PropertyValue(val, true);
        }
        
        private Hashtable _properties;
    }
}
