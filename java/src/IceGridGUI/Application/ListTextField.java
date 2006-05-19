// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import IceGrid.*;
import IceGridGUI.*;

import javax.swing.JTextField;

//
// A special field used to show/edit a list of strings separated
// by whitespace
//

public class ListTextField extends JTextField
{
    public ListTextField(int columns)
    {
	super(columns);
    }

    public void setList(java.util.List list, Utils.Resolver resolver)
    {
	java.util.Iterator p = list.iterator();
	String text = null;
	
	while(p.hasNext())
	{
	    if(text == null)
	    {
		text = "";
	    }
	    else
	    {
		text += " ";
	    }

	    String str = (String)p.next();
	    str = Utils.substitute(str, resolver);

	    if(str.length() == 0)
	    {
		text += "\"\"";
	    }
	    else if(str.matches("\\S*"))
	    {
		//
		// Only non-whitespace characters
		//
		text += str;
	    }
	    else
	    {
		text += '"' + str + '"';
	    }
	}
	setText(text);
    }

    public java.util.LinkedList getList()
    {
	String text = getText().trim();
	java.util.LinkedList result = new java.util.LinkedList();

	while(text.length() > 0)
	{
	    if(text.startsWith("\""))
	    {
		int last = text.indexOf("\"", 1);
		if(last == -1)
		{
		    result.add(text.substring(1));
		    text = "";
		}
		else
		{
		    result.add(text.substring(1, last));
		    text = text.substring(last + 1).trim();
		}
	    }
	    else
	    {
		String[] strings = text.split("\\s", 2);
		if(strings.length == 1)
		{
		    result.add(strings[0]);
		    text = "";
		}
		else
		{
		    result.add(strings[0]);
		    text = strings[1].trim();
		}
	    }
	}
	return result;
    }
}
