// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
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
        for(int i = 0; i < _factories.size(); i++)
        {
            EndpointFactory f = (EndpointFactory)_factories.get(i);
            if(f.type() == factory.type())
            {
                assert(false);
            }
        }
        _factories.add(factory);
    }

    public synchronized EndpointFactory
    get(short type)
    {
        for(int i = 0; i < _factories.size(); i++)
        {
            EndpointFactory f = (EndpointFactory)_factories.get(i);
            if(f.type() == type)
            {
                return f;
            }
        }
        return null;
    }

    public synchronized EndpointI
    create(String str, boolean adapterEndp)
    {
        String s = str.trim();
        if(s.length() == 0)
        {
	    Ice.EndpointParseException e = new Ice.EndpointParseException();
	    e.str = str;
	    throw e;
        }

        java.util.regex.Pattern p = java.util.regex.Pattern.compile("([ \t\n\r]+)|$");
        java.util.regex.Matcher m = p.matcher(s);
        boolean b = m.find();
        assert(b);

        String protocol = s.substring(0, m.start());

        if(protocol.equals("default"))
        {
            protocol = _instance.defaultsAndOverrides().defaultProtocol;
        }

        for(int i = 0; i < _factories.size(); i++)
        {
            EndpointFactory f = (EndpointFactory)_factories.get(i);
            if(f.protocol().equals(protocol))
            {
                return f.create(s.substring(m.end()), adapterEndp);
            }
        }

	return null;
    }

    public synchronized EndpointI
    read(BasicStream s)
    {
        EndpointI v;
        short type = s.readShort();

        for(int i = 0; i < _factories.size(); i++)
        {
            EndpointFactory f = (EndpointFactory)_factories.get(i);
            if(f.type() == type)
            {
                return f.read(s);
            }
        }

        return new UnknownEndpointI(type, s);
    }

    void
    destroy()
    {
        for(int i = 0; i < _factories.size(); i++)
        {
            EndpointFactory f = (EndpointFactory)_factories.get(i);
            f.destroy();
        }
        _factories.clear();
    }

    private Instance _instance;
    private java.util.ArrayList _factories = new java.util.ArrayList();
}
