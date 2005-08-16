// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class EndpointFactoryManager
{
    EndpointFactoryManager(Instance instance)
    {
        _instance = instance;
    }

    public synchronized void
    add(EndpointFactory factory)
    {
	java.util.Enumeration e = _factories.elements();
	while(e.hasMoreElements())
        {
            EndpointFactory f = (EndpointFactory)e.nextElement();
            if(f.type() == factory.type())
            {
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(false);
		}
            }
        }
        _factories.addElement(factory);
    }

    public synchronized EndpointFactory
    get(short type)
    {
	java.util.Enumeration e = _factories.elements();
	while(e.hasMoreElements())
        {
            EndpointFactory f = (EndpointFactory)e.nextElement();
            if(f.type() == type)
            {
                return f;
            }
        }
        return null;
    }

    public synchronized Endpoint
    create(String str)
    {
        String s = str.trim();
        if(s.length() == 0)
        {
	    Ice.EndpointParseException e = new Ice.EndpointParseException();
	    e.str = str;
	    throw e;
        }

	String protocol;
	String args;
	final String delim = " \t\n\r";
	int pos = IceUtil.StringUtil.findFirstOf(s, delim);
	if(pos == -1)
	{
	    protocol = s;
	    args = "";
	}
	else
	{
	    protocol = s.substring(0, pos);
	    int beg = IceUtil.StringUtil.findFirstNotOf(s, delim, pos);
	    if(beg == -1)
	    {
		args = "";
	    }
	    else
	    {
		args = s.substring(beg);
	    }
	}

        if(protocol.equals("default"))
        {
            protocol = _instance.defaultsAndOverrides().defaultProtocol;
        }

	java.util.Enumeration e = _factories.elements();
	while(e.hasMoreElements())
        {
            EndpointFactory f = (EndpointFactory)e.nextElement();
            if(f.protocol().equals(protocol))
            {
		return f.create(args);
            }
        }

	return null;
    }

    public synchronized Endpoint
    read(BasicStream s)
    {
        Endpoint v;
        short type = s.readShort();

	java.util.Enumeration e = _factories.elements();
	while(e.hasMoreElements())
	{
            EndpointFactory f = (EndpointFactory)e.nextElement();
            if(f.type() == type)
            {
                return f.read(s);
            }
        }

        return new UnknownEndpoint(type, s);
    }

    void
    destroy()
    {
	java.util.Enumeration e = _factories.elements();
	while(e.hasMoreElements())
	{
	    EndpointFactory f = (EndpointFactory)e.nextElement();
	    f.destroy();
	}
        _factories.removeAllElements();
    }

    private Instance _instance;
    private java.util.Vector _factories = new java.util.Vector();
}
