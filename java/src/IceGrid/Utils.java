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

}
