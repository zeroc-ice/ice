// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text.RegularExpressions;
using System.Globalization;
using Microsoft.Win32;

namespace Ice
{
    sealed class PropertiesI :  Properties
    {
        class PropertyValue
        {
            public PropertyValue(PropertyValue v)
            {
                val = v.val;
                used = v.used;
            }

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
                PropertyValue pv;
                if(_properties.TryGetValue(key, out pv))
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
                PropertyValue pv;
                if(_properties.TryGetValue(key, out pv))
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
                PropertyValue pv;
                if(!_properties.TryGetValue(key, out pv))
                {
                    return val;
                }
                pv.used = true;
                try
                {
                    return int.Parse(pv.val, CultureInfo.InvariantCulture);
                }
                catch(FormatException)
                {
                    Util.getProcessLogger().warning("numeric property " + key +
                                                    " set to non-numeric value, defaulting to " + val);
                    return val;
                }
            }
        }

        public string[] getPropertyAsList(string key)
        {
            return getPropertyAsListWithDefault(key, null);
        }

        public string[] getPropertyAsListWithDefault(string key, string[] val)
        {
            if(val == null)
            {
                val = new string[0];
            }

            lock(this)
            {
                PropertyValue pv;
                if(!_properties.TryGetValue(key, out pv))
                {
                    return val;
                }

                pv.used = true;

                string[] result = IceUtilInternal.StringUtil.splitString(pv.val, ", \t\r\n");
                if(result == null)
                {
                    Util.getProcessLogger().warning("mismatched quotes in property " + key
                                                    + "'s value, returning default value");
                    return val;
                }
                else
                {
                    return result;
                }
            }
        }

        public Dictionary<string, string> getPropertiesForPrefix(string prefix)
        {
            lock(this)
            {
                Dictionary<string, string> result = new Dictionary<string, string>();

                foreach(string s in _properties.Keys)
                {
                    if(prefix.Length == 0 || s.StartsWith(prefix, StringComparison.Ordinal))
                    {
                        PropertyValue pv = _properties[s];
                        pv.used = true;
                        result[s] = pv.val;
                    }
                }
                return result;
            }
        }

        public void setProperty(string key, string val)
        {
            //
            // Trim whitespace
            //
            if(key != null)
            {
                key = key.Trim();
            }
            if(key == null || key.Length == 0)
            {
                throw new InitializationException("Attempt to set property with empty key");
            }

            //
            // Check if the property is legal.
            //
            Logger logger = Util.getProcessLogger();
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
                    bool mismatchCase = false;
                    string otherKey = "";
                    if(!propPrefix.ToUpper().Equals(prefix.ToUpper()))
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

                        if(!found)
                        {
                            r = new Regex(IceInternal.PropertyNames.validProps[i][j].pattern().ToUpper());
                            m = r.Match(key.ToUpper());
                            if(m.Success)
                            {
                                found = true;
                                mismatchCase = true;
                                otherKey = IceInternal.PropertyNames.validProps[i][j].pattern().Replace("\\", "").
                                                                                                Replace("^", "").
                                                                                                Replace("$", "");
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

            lock(this)
            {
                //
                //
                // Set or clear the property.
                //
                if(val != null && val.Length > 0)
                {
                    PropertyValue pv;
                    if(_properties.TryGetValue(key, out pv))
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
                foreach(KeyValuePair<string, PropertyValue> entry in _properties)
                {
                    result[i++] = "--" + entry.Key + "=" + entry.Value.val;
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

            List<string> result = new List<string>();
            for(int i = 0; i < options.Length; i++)
            {
                string opt = options[i];
                if(opt.StartsWith(pfx, StringComparison.Ordinal))
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
#if NET45
            if(file.StartsWith("HKCU\\", StringComparison.Ordinal) ||
               file.StartsWith("HKLM\\", StringComparison.Ordinal))
            {
                RegistryKey key =
                    file.StartsWith("HKCU\\", StringComparison.Ordinal) ? Registry.CurrentUser : Registry.LocalMachine;
                RegistryKey iceKey = key.OpenSubKey(file.Substring(file.IndexOf("\\") + 1));
                if(iceKey == null)
                {
                    Ice.InitializationException ex = new Ice.InitializationException();
                    ex.reason = "Could not open Windows registry key `" + file + "'";
                    throw ex;
                }

                foreach(string propKey in iceKey.GetValueNames())
                {
                    RegistryValueKind kind = iceKey.GetValueKind(propKey);
                    if(kind == RegistryValueKind.String || kind == RegistryValueKind.ExpandString)
                    {
                        setProperty(propKey, iceKey.GetValue(propKey).ToString());
                    }
                }
            }
            else
            {
#endif
                try
                {
                    using(System.IO.StreamReader sr = new System.IO.StreamReader(file))
                    {
                        parse(sr);
                    }
                }
                catch(System.IO.IOException ex)
                {
                    FileException fe = new FileException(ex);
                    fe.path = file;
                    throw fe;
                }
#if NET45
            }
#endif
        }

        public Properties ice_clone_()
        {
            lock(this)
            {
                return new PropertiesI(this);
            }
        }

        public List<string> getUnusedProperties()
        {
            lock(this)
            {
                List<string> unused = new List<string>();
                foreach(KeyValuePair<string, PropertyValue> entry in _properties)
                {
                    if(!entry.Value.used)
                    {
                        unused.Add(entry.Key);
                    }
                }
                return unused;
            }
        }

        internal PropertiesI(PropertiesI p)
        {
            //
            // NOTE: we can't just do a shallow copy of the map as the map values
            // would otherwise be shared between the two PropertiesI object.
            //
            //_properties = new Dictionary<string, PropertyValue>(p._properties);
            _properties = new Dictionary<string, PropertyValue>();
            foreach(KeyValuePair<string, PropertyValue> entry in p._properties)
            {
                _properties[entry.Key] = new PropertyValue(entry.Value);
            }
        }

        internal PropertiesI()
        {
            _properties = new Dictionary<string, PropertyValue>();
        }

        internal PropertiesI(ref string[] args, Properties defaults)
        {
            if(defaults == null)
            {
                _properties = new Dictionary<string, PropertyValue>();
            }
            else
            {
                //
                // NOTE: we can't just do a shallow copy of the map as the map values
                // would otherwise be shared between the two PropertiesI object.
                //
                //_properties = ((PropertiesI)defaults)._properties;
                _properties = new Dictionary<string, PropertyValue>();
                foreach(KeyValuePair<string, PropertyValue> entry in ((PropertiesI)defaults)._properties)
                {
                    _properties[entry.Key] = new PropertyValue(entry.Value);
                }
            }

            PropertyValue pv;
            if(_properties.TryGetValue("Ice.ProgramName", out pv))
            {
                pv.used = true;
            }
            else
            {
                _properties["Ice.ProgramName"] = new PropertyValue(AppDomain.CurrentDomain.FriendlyName, true);
            }

            bool loadConfigFiles = false;

            for(int i = 0; i < args.Length; i++)
            {
                if(args[i].StartsWith("--Ice.Config", StringComparison.Ordinal))
                {
                    string line = args[i];
                    if(line.IndexOf('=') == -1)
                    {
                        line += "=1";
                    }
                    parseLine(line.Substring(2));
                    loadConfigFiles = true;

                    string[] arr = new string[args.Length - 1];
                    Array.Copy(args, 0, arr, 0, i);
                    if(i < args.Length - 1)
                    {
                        Array.Copy(args, i + 1, arr, i, args.Length - i - 1);
                    }
                    args = arr;
                }
            }

            if(!loadConfigFiles)
            {
                //
                // If Ice.Config is not set, load from ICE_CONFIG (if set)
                //
                loadConfigFiles = !_properties.ContainsKey("Ice.Config");
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

        private const int ParseStateKey = 0;
        private const int ParseStateValue = 1;

        private void parseLine(string line)
        {
            string key = "";
            string val = "";

            int state = ParseStateKey;

            string whitespace = "";
            string escapedspace = "";
            bool finished = false;
            for(int i = 0; i < line.Length; ++i)
            {
                char c = line[i];
                switch(state)
                {
                  case ParseStateKey:
                  {
                      switch(c)
                      {
                        case '\\':
                          if(i < line.Length - 1)
                          {
                              c = line[++i];
                              switch(c)
                              {
                                case '\\':
                                case '#':
                                case '=':
                                  key += whitespace;
                                  whitespace= "";
                                  key += c;
                                  break;

                                case ' ':
                                  if(key.Length != 0)
                                  {
                                      whitespace += c;
                                  }
                                  break;

                                default:
                                  key += whitespace;
                                  whitespace= "";
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
                            if(key.Length != 0)
                            {
                                whitespace += c;
                            }
                            break;

                        case '=':
                            whitespace= "";
                            state = ParseStateValue;
                            break;

                        case '#':
                            finished = true;
                            break;

                        default:
                            key += whitespace;
                            whitespace= "";
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
                          if(i < line.Length - 1)
                          {
                              c = line[++i];
                              switch(c)
                              {
                                case '\\':
                                case '#':
                                case '=':
                                  val += val.Length == 0 ? escapedspace : whitespace;
                                  whitespace= "";
                                  escapedspace= "";
                                  val += c;
                                  break;

                                case ' ':
                                  whitespace += c;
                                  escapedspace += c;
                                  break;

                                default:
                                  val += val.Length == 0 ? escapedspace : whitespace;
                                  whitespace= "";
                                  escapedspace= "";
                                  val += '\\';
                                  val += c;
                                  break;
                              }
                          }
                          else
                          {
                              val += val.Length == 0 ? escapedspace : whitespace;
                              val += c;
                          }
                          break;

                        case ' ':
                        case '\t':
                        case '\r':
                        case '\n':
                            if(val.Length != 0)
                            {
                                whitespace += c;
                            }
                            break;

                        case '#':
                            finished = true;
                            break;

                        default:
                            val += val.Length == 0 ? escapedspace : whitespace;
                            whitespace = "";
                            escapedspace = "";
                            val += c;
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
            val += escapedspace;

            if((state == ParseStateKey && key.Length != 0) || (state == ParseStateValue && key.Length == 0))
            {
                Util.getProcessLogger().warning("invalid config file entry: \"" + line + "\"");
                return;
            }
            else if(key.Length == 0)
            {
                return;
            }

            setProperty(key, val);
        }

        private void loadConfig()
        {
            string val = getProperty("Ice.Config");
            if(val.Length == 0 || val.Equals("1"))
            {
                string s = Environment.GetEnvironmentVariable("ICE_CONFIG");
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
                    load(files[i].Trim());
                }

                _properties["Ice.Config"] = new PropertyValue(val, true);
            }
        }

        private Dictionary<string, PropertyValue> _properties;
    }
}
