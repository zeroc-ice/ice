// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public class JADTask extends org.apache.tools.ant.Task
{
    public
    JADTask()
    {
    }

    public void
    setFile(java.io.File jadFile)
    {
	_jad = jadFile;
    }

    public void
    addConfiguredAttribute(JADAttribute attribute)
    {
	_attributes.add(attribute);
    }

    public void
    execute()
	throws org.apache.tools.ant.BuildException
    {
	if(_jad == null)
	{
	    throw new org.apache.tools.ant.BuildException("No JAD filename set.");
	}

	if(_jad.exists() && !_jad.canWrite())
	{
	    throw new org.apache.tools.ant.BuildException("Cannot write to " + _jad.getName());
	}

	try
	{
	    java.io.PrintWriter out = new java.io.PrintWriter(new java.io.FileWriter(_jad));
	    java.util.Iterator iter = _attributes.iterator();
	    while(iter.hasNext())
	    {
		JADAttribute attr = (JADAttribute)iter.next();
		out.println(attr.getName() + ": " + attr.getValue());
	    }
	    out.flush();
	    out.close();
	}
	catch(java.io.IOException ex)
	{
	    throw new org.apache.tools.ant.BuildException(ex.getMessage());
	}
    }

    private java.io.File _jad;
    private java.util.List _attributes = new java.util.LinkedList();
}
