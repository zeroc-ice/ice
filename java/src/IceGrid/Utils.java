// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;

import javax.swing.ImageIcon;

public class Utils
{
    static public ImageIcon getIcon(String path)
    {
	java.net.URL imgURL = Utils.class.getResource(path);
	assert(imgURL != null);
	return new ImageIcon(imgURL);
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

		result += elt;
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


    //
    // Same algorithm as DescriptorVariables::substituteImpl (see IceGrid C++)
    //
    static public String substituteVariables(String input, java.util.Map[] stringMaps)
    {
	if(stringMaps == null)
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
		while(escape > 0 && input.charAt(escape = 1) == '$')
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
		// Malformed variable, can't subsitute anything else
		//
		return input;
	    }

	    String name = input.substring(beg + 2, end);
	    
	    //
	    // Lookup name's value
	    //
	    String val = null;
	    for(int i = 0; i < stringMaps.length; ++i)
	    {
		if(stringMaps[i] != null)
		{
		    val = (String)stringMaps[i].get(name);
		    if(val != null)
		    {
			break; // for
		    }
		}
	    }
	    
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

    static public String substituteVariables(String input, java.util.Map m1)
    {
	return substituteVariables(input,
				   new java.util.Map[]{m1});
    }

    static public String substituteVariables(String input, java.util.Map m1,
					    java.util.Map m2)
    {
	return substituteVariables(input, 
				   new java.util.Map[]{m1, m2});
    }
    
    static public String substituteVariables(String input, java.util.Map m1,
					     java.util.Map m2, java.util.Map m3)
    {
	return substituteVariables(input, 
				   new java.util.Map[]{m1, m2, m3});
    }

    static public java.util.Map substituteVariables(java.util.Map input, 
						    java.util.Map[] stringMaps)
    {
	java.util.Map result = new java.util.HashMap();

	java.util.Iterator p = input.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    result.put(entry.getKey(), 
		       substituteVariables((String)entry.getValue(), stringMaps));
	}

	return result;
    }


    static public java.util.Map substituteVariables(java.util.Map input, 
						    java.util.Map m1,
						    java.util.Map m2)
    {
	return substituteVariables(input, 
				   new java.util.Map[]{m1, m2});
    }

    static public java.util.Map substituteVariables(java.util.Map input, 
						    java.util.Map m1,
						    java.util.Map m2,
						    java.util.Map m3)
    {
	return substituteVariables(input,
				   new java.util.Map[]{m1, m2, m3});
    }


    static public class Resolver
    {
	//
	// Simple resolver
	//
	public Resolver(java.util.Map[] variables)
	{
	    _variables = variables;
	    _resolvedVariableCache = new java.util.HashMap();
	    _parameters = null;
	    _subResolver = this;
	}

	//
	// Resolver for instance; parameters is not yet substituted
	//
	public Resolver(Resolver parent, java.util.Map parameters,
			java.util.Map defaults)
	{
	    java.util.Map substitutedParameters = 
		parent.substituteParameterValues(parameters, defaults);

	    _variables = parent._variables;
	    _resolvedVariableCache = 
		new java.util.HashMap(parent._resolvedVariableCache);
	    _parameters = substitutedParameters;
	    _subResolver = new Resolver(_variables, _resolvedVariableCache);
	}
	
	//
	// Resolver for plain server
	//
	public Resolver(Resolver parent)
	{
	    _variables = parent._variables;
	    _resolvedVariableCache = 
		new java.util.HashMap(parent._resolvedVariableCache);
	    _parameters = parent._parameters;
	    if(_parameters == null)
	    {
		_subResolver = this;
	    }
	    else
	    {
		_subResolver = new Resolver(_variables, _resolvedVariableCache);
	    }
	}

	private Resolver(java.util.Map[] variables,
			 java.util.Map resolvedVariableCache)
	{
	    _variables = variables;
	    _resolvedVariableCache = resolvedVariableCache;
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
	  
	    Object obj = _resolvedVariableCache.get(name);
	    if(obj != null)
	    {
		return (String)obj;
	    }
	    else
	    {
		for(int i = 0; i < _variables.length; ++i)
		{
		    obj = _variables[i].get(name);
		    if(obj != null)
		    {
			break;
		    }
		} 
		if(obj != null)
		{
		    //
		    // If I lookup myself, replace xxx${myself}xxx by xxx<error msg>xxx
		    //
		    _resolvedVariableCache.put(name, _recursiveDefError);
		    String result = _subResolver.substitute((String)obj);
		    _resolvedVariableCache.put(name, result);
		    return result;
		}
		else
		{
		    return null;
		}
	    }
	}

	//
	// Put this entry; particularly useful for predefined variables
	//
	public void put(String name, String value)
	{
	    _resolvedVariableCache.put(name, value);
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
	    int beg = 0;
	    int end = 0;
	    
	    while((beg = input.indexOf("${", beg)) != -1)
	    {
		if(beg > 0 && input.charAt(beg - 1) == '$')
		{
		    int escape = beg - 1;
		    while(escape > 0 && input.charAt(escape = 1) == '$')
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
		if(result.get(entry.getKey()) == null)
		{
		    result.put(entry.getKey(), substitute((String)entry.getValue()));
		}
	    }
	    return result;
	}

	private java.util.Map[] _variables;
	private java.util.Map _resolvedVariableCache;
	private java.util.Map _parameters;
	private Resolver _subResolver;
	static private String _recursiveDefError = "<recursive def error>";
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
}
