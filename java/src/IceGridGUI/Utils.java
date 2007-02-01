// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI;

import javax.swing.ImageIcon;
import IceGrid.*;

public class Utils
{
    static public ImageIcon getIcon(String path)
    {
        java.net.URL imgURL = Utils.class.getResource(path);
        if(imgURL == null)
        {
            System.err.println("Could not find icon " + path);
            return null;
        }
        else
        {
            return new ImageIcon(imgURL);
        }
    }

    static public interface Stringifier
    {
        public String toString(Object obj);
    }

    //
    // Stringify helpers
    //
    static public String stringify(java.util.Collection col,
                                   Stringifier stringifier,
                                   String separator,
                                   Ice.StringHolder toolTipHolder)
    {
        String result = "";
        if(toolTipHolder != null)
        {
            toolTipHolder.value = null;
        }
     
        java.util.Iterator p = col.iterator();
        
        boolean firstElement = true;
        while(p.hasNext())
        {
            String elt = stringifier.toString(p.next());
            if(elt != null)
            {
                if(firstElement)
                {
                    firstElement = false;
                    if(toolTipHolder != null)
                    {
                        toolTipHolder.value = "<html>";
                    }
                }
                else
                {
                    result += separator;
                    if(toolTipHolder != null)
                    {
                        toolTipHolder.value += "<br>";
                    }
                }
                
                if(elt.length() == 0)
                {
                    result += "\"\"";
                }
                else if(elt.matches("\\S*"))
                {
                    //
                    // Only non-whitespace characters
                    //
                    result += elt;
                }
                else
                {
                    result += '"' + elt + '"';
                }

                if(toolTipHolder != null)
                {
                    toolTipHolder.value += elt;
                }
            }
        }
        if(toolTipHolder != null && toolTipHolder.value != null)
        {
            toolTipHolder.value += "</html>";
        }
        
        return result;  
    }
    
    static public String stringify(java.util.Collection col, 
                                   String separator,
                                   Ice.StringHolder toolTipHolder)
    {
        
        Stringifier stringifier = new Stringifier()
            {
                public String toString(Object obj)
                {
                    return (String)obj;
                }
            };
        return stringify(col, stringifier, separator, toolTipHolder);

    }

    static public String stringify(String[] stringSeq, String separator,
                                   Ice.StringHolder toolTipHolder)
    {

        return stringify(java.util.Arrays.asList(stringSeq), separator, toolTipHolder);
    }

    static public String stringify(java.util.Map stringMap, 
                                   final String pairSeparator,
                                   String separator,
                                   Ice.StringHolder toolTipHolder)
    {
        Stringifier stringifier = new Stringifier()
            {
                public String toString(Object obj)
                {
                    java.util.Map.Entry entry = (java.util.Map.Entry)obj;
                    return (String)entry.getKey() + pairSeparator + (String)entry.getValue();
                }
            };

        return stringify(stringMap.entrySet(), stringifier, separator, toolTipHolder);
    }

    static public class Resolver
    {
        //
        // Application-level resolver
        //
        public Resolver(java.util.Map variables)
        {
            this(new java.util.Map[]{variables});
        }
      
        public Resolver(java.util.Map[] variables)
        {
            _variables = variables;
            _predefinedVariables = new java.util.HashMap();
         
            _parameters = null;
            _subResolver = this;
        }

        //
        // Resolver for instance; in-parameters are not yet substituted
        //
        public Resolver(Resolver parent, java.util.Map parameters,
                        java.util.Map defaults)
        {
            _variables = parent._variables;

            //
            // Whenever the parent's predefined variables change, the resolver
            // must be recreated
            //
            reset(parent, parameters, defaults);
        }
        
        //
        // Resolver for plain server or service
        //
        public Resolver(Resolver parent)
        {
            _variables = parent._variables;
            _predefinedVariables = new java.util.HashMap(parent._predefinedVariables);
            _parameters = parent._parameters;
            if(_parameters == null)
            {
                _subResolver = this;
            }
            else
            {
                _subResolver = new Resolver(_variables, _predefinedVariables);
            }
        }

        private Resolver(java.util.Map[] variables,
                         java.util.Map predefinedVariables)
        {
            _variables = variables;
            _predefinedVariables = _predefinedVariables;

            _parameters = null;
            _subResolver = this;
        }       

        public String find(String name)
        {
            if(_parameters != null)
            {
                Object obj = _parameters.get(name);
                if(obj != null)
                {
                    return (String)obj;
                }
            }

            Object obj = _predefinedVariables.get(name);
            if(obj != null)
            {
                return (String)obj;
            }
          
            for(int i = 0; i < _variables.length; ++i)
            {
                obj = _variables[i].get(name);
                if(obj != null)
                {
                    return _subResolver.substitute((String)obj);
                }
            }
            return null;
        }

        //
        // Set a pre-defined variable; returns true if value was updated
        //
        public boolean put(String name, String value)
        {
            String oldVal = (String)_predefinedVariables.get(name);
            if(oldVal == null || !oldVal.equals(value))
            {
                _predefinedVariables.put(name, value);
                return true;
            }
            else
            {
                return false;
            }
        }

        //
        // Reset parameters and pre-defined variables
        //
        public void reset(Resolver parent, java.util.Map parameters,
                          java.util.Map defaults)
        {
            assert _variables == parent._variables;
            _predefinedVariables = new java.util.HashMap(parent._predefinedVariables);
           
            _parameters = parent.substituteParameterValues(parameters, defaults);
            _subResolver = new Resolver(_variables, _predefinedVariables);
        }

        public void reset(Resolver parent)
        {
            assert _variables == parent._variables;
            _predefinedVariables = new java.util.HashMap(parent._predefinedVariables);
           
            assert _parameters == parent._parameters;
            if(_parameters == null)
            {
                _subResolver = this;
            }
            else
            {
                _subResolver = new Resolver(_variables, _predefinedVariables);
            }
        }

        //
        // The sorted substituted parameters
        //
        public java.util.Map getParameters()
        {
            return _parameters;
        }

        public String substitute(String input)
        {
            if(input == null)
            {
                return input;
            }

            int beg = 0;
            int end = 0;
            
            while((beg = input.indexOf("${", beg)) != -1)
            {
                if(beg > 0 && input.charAt(beg - 1) == '$')
                {
                    int escape = beg - 1;
                    while(escape > 0 && input.charAt(escape - 1) == '$')
                    {
                        --escape;
                    }
                    
                    input = input.substring(0, escape) + input.substring(beg - (beg - escape) / 2);
                    if((beg - escape) % 2 != 0)
                    {
                        ++beg;
                        continue;
                    }
                    else
                    {
                        beg -= (beg - escape) / 2;
                    }
                }
                
                end = input.indexOf('}', beg);
                if(end == -1)
                {
                    //
                    // Malformed variable, can't substitute anything else
                    //
                    return input;
                }
                
                String name = input.substring(beg + 2, end);
                
                //
                // Resolve name
                //
                String val = find(name);  
                if(val != null)
                {
                    input = input.substring(0, beg) + val + input.substring(end + 1);
                    beg += val.length();
                }
                else
                {
                    // 
                    // No substitution, keep ${name} in the result
                    //
                    ++beg;
                }
            }
            return input;
        }

        //
        // Substitute all the values from the input map
        //
        public java.util.Map substituteParameterValues(java.util.Map input,
                                                       java.util.Map defaults)
        {
            java.util.Map result = new java.util.HashMap();
            java.util.Iterator p = input.entrySet().iterator();
            while(p.hasNext())
            {
                java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
                result.put(entry.getKey(), substitute((String)entry.getValue()));
            }
            p = defaults.entrySet().iterator();
            while(p.hasNext())
            {
                java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
                if(!result.containsKey(entry.getKey()))
                {
                    result.put(entry.getKey(), substitute((String)entry.getValue()));
                }
            }
            return result;
        }

        private java.util.Map[] _variables;
        private java.util.Map _parameters;
        private java.util.Map _predefinedVariables;

        private Resolver _subResolver;
    }

    
    static public String substitute(String input, Resolver resolver)
    {
        if(resolver != null)
        {
            return resolver.substitute(input);
        }
        else
        {
            return input;
        }
    }

    //
    // An expanded property set (i.e. containing other property sets)
    //
    static public class ExpandedPropertySet
    {
        public ExpandedPropertySet[] references;
        public java.util.List properties;       // list of PropertyDescriptor 
    }

    static public java.util.SortedMap propertySetsToMap(
        java.util.List propertySets,
        Resolver resolver)
    {
        java.util.SortedMap toMap = new java.util.TreeMap();
        java.util.Iterator p = propertySets.iterator();
        while(p.hasNext())
        {
            ExpandedPropertySet propertySet = (ExpandedPropertySet)p.next();
            addSet(propertySet, resolver, toMap);
        }
        return toMap;
    }

    static public java.util.SortedMap propertySetToMap(
        ExpandedPropertySet propertySet,
        Resolver resolver)
    {
        java.util.List list = new java.util.LinkedList();
        list.add(propertySet);
        return propertySetsToMap(list, resolver);
    }

    static private void addSet(ExpandedPropertySet set, Resolver resolver,
                               java.util.SortedMap toMap)
    {
        for(int i = 0; i < set.references.length; ++i)
        {
            addSet(set.references[i], resolver, toMap);
        }
        
        java.util.Iterator p = set.properties.iterator();
        while(p.hasNext())
        {
            PropertyDescriptor pd = (PropertyDescriptor)p.next();
            String name = (String)pd.name;
            String val = (String)pd.value;
           
            name = substitute(pd.name, resolver);
            val = substitute(pd.value, resolver);
           
            toMap.put(name, val);
        }
    }
}
