// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
            EndpointFactory f = _factories.get(i);
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
            EndpointFactory f = _factories.get(i);
            if(f.type() == type)
            {
                return f;
            }
        }
        return null;
    }

    public synchronized EndpointI
    create(String str, boolean oaEndpoint)
    {
        String s = str.trim();
        if(s.length() == 0)
        {
            Ice.EndpointParseException e = new Ice.EndpointParseException();
            e.str = "value has no non-whitespace characters";
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
            EndpointFactory f = _factories.get(i);
            if(f.protocol().equals(protocol))
            {
                return f.create(s.substring(m.end()), oaEndpoint);

                // Code below left in place for debugging.

                /*
                EndpointI e = f.create(s.substring(m.end()), oaEndpoint);
                BasicStream bs = new BasicStream(_instance, true, false);
                e.streamWrite(bs);
                java.nio.ByteBuffer buf = bs.getBuffer();
                buf.position(0);
                short type = bs.readShort();
                EndpointI ue = new IceInternal.OpaqueEndpointI(type, bs);
                System.err.println("Normal: " + e);
                System.err.println("Opaque: " + ue);
                return e;
                */
            }
        }

        //
        // If the stringified endpoint is opaque, create an unknown endpoint,
        // then see whether the type matches one of the known endpoints.
        //
        if(protocol.equals("opaque"))
        {
            EndpointI ue = new OpaqueEndpointI(s.substring(m.end()));
            for(int i = 0; i < _factories.size(); i++)
            {
                EndpointFactory f = _factories.get(i);
                if(f.type() == ue.type())
                {
                    //
                    // Make a temporary stream, write the opaque endpoint data into the stream,
                    // and ask the factory to read the endpoint data from that stream to create
                    // the actual endpoint.
                    //
                    BasicStream bs = new BasicStream(_instance, Protocol.currentProtocolEncoding, true, false);
                    ue.streamWrite(bs);
                    Buffer buf = bs.getBuffer();
                    buf.b.position(0);
                    bs.readShort(); // type
                    return f.read(bs);
                }
            }
            return ue; // Endpoint is opaque, but we don't have a factory for its type.
        }

        return null;
    }

    public synchronized EndpointI
    read(BasicStream s)
    {
        short type = s.readShort();

        for(int i = 0; i < _factories.size(); i++)
        {
            EndpointFactory f = _factories.get(i);
            if(f.type() == type)
            {
                return f.read(s);
            }
        }
        return new OpaqueEndpointI(type, s);
    }

    void
    destroy()
    {
        for(int i = 0; i < _factories.size(); i++)
        {
            EndpointFactory f = _factories.get(i);
            f.destroy();
        }
        _factories.clear();
    }

    private Instance _instance;
    private java.util.List<EndpointFactory> _factories = new java.util.ArrayList<EndpointFactory>();
}
