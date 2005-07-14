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
	toolTipHolder.value = null;
	java.util.Iterator p = col.iterator();
	
	while(p.hasNext())
	{
	    if(result.length() > 0)
	    {
		result += separator;
		toolTipHolder.value += "<br>";
	    }
	    else
	    {
		toolTipHolder.value = "<html>";
	    }
	    String elt = stringifier.toString(p.next());
	    result += elt;
	    toolTipHolder.value += elt;
	}
	if(toolTipHolder.value != null)
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
}
