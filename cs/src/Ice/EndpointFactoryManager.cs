// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections;
    using System.Diagnostics;
    using System.Text.RegularExpressions;

    public sealed class EndpointFactoryManager
    {
	internal EndpointFactoryManager(Instance instance)
	{
	    instance_ = instance;
	    _factories = new ArrayList();
	}
	
	public void add(EndpointFactory factory)
	{
	    lock(this)
	    {
		for(int i = 0; i < _factories.Count; i++)
		{
		    EndpointFactory f = (EndpointFactory)_factories[i];
		    if(f.type() == factory.type())
		    {
			Debug.Assert(false);
		    }
		}
		_factories.Add(factory);
	    }
	}
	
	public EndpointFactory get(short type)
	{
	    lock(this)
	    {
		for(int i = 0; i < _factories.Count; i++)
		{
		    EndpointFactory f = (EndpointFactory)_factories[i];
		    if(f.type() == type)
		    {
			return f;
		    }
		}
		return null;
	    }
	}
	
	public EndpointI create(string str, bool adapterEndp)
	{
	    lock(this)
	    {
		string s = str.Trim();
		if(s.Length == 0)
		{
		    Ice.EndpointParseException e = new Ice.EndpointParseException();
		    e.str = str;
		    throw e;
		}
		
		Regex p = new Regex("([ \t\n\r]+)|$");
		Match m = p.Match(s);
		Debug.Assert(m.Success);
		
		string protocol = s.Substring(0, m.Index);
		
		if(protocol.Equals("default"))
		{
		    protocol = instance_.defaultsAndOverrides().defaultProtocol;
		}
		
		for(int i = 0; i < _factories.Count; i++)
		{
		    EndpointFactory f = (EndpointFactory)_factories[i];
		    if(f.protocol().Equals(protocol))
		    {
			return f.create(s.Substring(m.Index + m.Length), adapterEndp);
		    }
		}
		
		return null;
	    }
	}
	
	public EndpointI read(BasicStream s)
	{
	    lock(this)
	    {
		short type = s.readShort();
		
		for(int i = 0; i < _factories.Count; i++)
		{
		    EndpointFactory f = (EndpointFactory)_factories[i];
		    if(f.type() == type)
		    {
			return f.read(s);
		    }
		}
		
		return new UnknownEndpointI(type, s);
	    }
	}
	
	internal void destroy()
	{
	    for(int i = 0; i < _factories.Count; i++)
	    {
		EndpointFactory f = (EndpointFactory)_factories[i];
		f.destroy();
	    }
	    _factories.Clear();
	}
	
	private readonly Instance instance_;
	private readonly ArrayList _factories;
    }

}
