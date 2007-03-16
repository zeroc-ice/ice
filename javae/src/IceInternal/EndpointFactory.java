// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class EndpointFactory
{
    EndpointFactory(Instance instance)
    {
        _instance = instance;
    }

    public Endpoint
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

        if(protocol.equals("default") || protocol.equals("tcp"))
        {
            return new TcpEndpoint(_instance, args);
        }

        return null;
    }

    public Endpoint
    read(BasicStream s)
    {
        Endpoint v;
        short type = s.readShort();

	if(type == TcpEndpoint.TYPE)
	{
            return new TcpEndpoint(s);
	}

        return new UnknownEndpoint(type, s);
    }

    public void
    destroy()
    {
        _instance = null;
    }

    private Instance _instance;
}
