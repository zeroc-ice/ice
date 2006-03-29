// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI;

import java.io.*;

//
// Helper class to write XML files
//
public class XMLWriter
{
    public XMLWriter(File file) throws FileNotFoundException, IOException
    {
	try
	{
	    _writer = new BufferedWriter(
		new OutputStreamWriter(new FileOutputStream(file), "UTF-8"));
	}
	catch(UnsupportedEncodingException e)
	{
	    assert false;
	}
	_writer.write("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" + _newline);
	_writer.write("<!-- This file was written by IceGrid Admin -->" + _newline);
    }

    public void writeElement(String name, java.util.List attributes)
	throws IOException
    {
	_writer.write(_indent);
	_writer.write("<" + name);
	writeAttributes(attributes);
	_writer.write("/>" + _newline);
    }

    public void writeElement(String name) throws IOException
    {
	writeElement(name, (java.util.List)null);
    }

    public void writeElement(String name, String content) throws IOException
    {
	//
	// TODO: deal with ]]> content
	//
	_writer.write(_indent);
	_writer.write("<" + name + "><![CDATA[" + content 
		      + "]]></" + name + ">" + _newline);
    }

    public void writeStartTag(String name, java.util.List attributes)
	throws IOException
    {
	_writer.write(_indent);
	_writer.write("<" + name);
	writeAttributes(attributes);
	_writer.write(">");
	_writer.write(_newline);
	increaseIndent();
    }

    public void writeStartTag(String name) throws IOException
    {
	writeStartTag(name, null);
    }

    public void writeEndTag(String name) throws IOException
    {
	decreaseIndent();
	_writer.write(_indent);
	_writer.write("</" + name + ">" + _newline);
    }

    public void close()  throws IOException
    {
	_writer.close();
    }

    public void flush() throws IOException
    {
	_writer.flush();
    }

    private void writeAttributes(java.util.List attributes)
	throws IOException
    {
	if(attributes != null)
	{
	    java.util.Iterator p = attributes.iterator();
	    while(p.hasNext())
	    {
		String[] pair = (String[])p.next();
		_writer.write(" " + pair[0] + "=\"" + pair[1] + "\"");
	    }
	}
    }

    private void increaseIndent()
    {
	_indent += "   ";
    }

    private void decreaseIndent()
    {
	_indent = _indent.substring(3);
    }

    private Writer _writer;
    private String _indent = "";
    private static String _newline = System.getProperty("line.separator");
}
