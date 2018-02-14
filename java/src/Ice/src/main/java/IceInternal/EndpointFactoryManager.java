// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

    public synchronized void add(EndpointFactory factory)
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

    public synchronized EndpointFactory get(short type)
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

    public synchronized EndpointI create(String str, boolean oaEndpoint)
    {
        String[] arr = IceUtilInternal.StringUtil.splitString(str, " \t\r\n");
        if(arr == null)
        {
            Ice.EndpointParseException e = new Ice.EndpointParseException();
            e.str = "mismatched quote";
            throw e;
        }

        if(arr.length == 0)
        {
            Ice.EndpointParseException e = new Ice.EndpointParseException();
            e.str = "value has no non-whitespace characters";
            throw e;
        }

        java.util.ArrayList<String> v = new java.util.ArrayList<String>(java.util.Arrays.asList(arr));
        String protocol = v.get(0);
        v.remove(0);

        if(protocol.equals("default"))
        {
            protocol = _instance.defaultsAndOverrides().defaultProtocol;
        }

        EndpointFactory factory = null;

        for(int i = 0; i < _factories.size(); i++)
        {
            EndpointFactory f = _factories.get(i);
            if(f.protocol().equals(protocol))
            {
                factory = f;
            }
        }

        if(factory != null)
        {
            EndpointI e = factory.create(v, oaEndpoint);
            if(!v.isEmpty())
            {
                Ice.EndpointParseException ex = new Ice.EndpointParseException();
                ex.str = "unrecognized argument `" + v.get(0) + "' in endpoint `" + str + "'";
                throw ex;
            }
            return e;

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

        //
        // If the stringified endpoint is opaque, create an unknown endpoint,
        // then see whether the type matches one of the known endpoints.
        //
        if(protocol.equals("opaque"))
        {
            EndpointI ue = new OpaqueEndpointI(v);
            if(!v.isEmpty())
            {
                Ice.EndpointParseException ex = new Ice.EndpointParseException();
                ex.str = "unrecognized argument `" + v.get(0) + "' in endpoint `" + str + "'";
                throw ex;
            }
            factory = get(ue.type());
            if(factory != null)
            {
                //
                // Make a temporary stream, write the opaque endpoint data into the stream,
                // and ask the factory to read the endpoint data from that stream to create
                // the actual endpoint.
                //
                BasicStream bs = new BasicStream(_instance, Protocol.currentProtocolEncoding, false);
                bs.writeShort(ue.type());
                ue.streamWrite(bs);
                Buffer buf = bs.getBuffer();
                buf.b.position(0);
                buf.b.limit(buf.size());
                bs.readShort(); // type
                bs.startReadEncaps();
                EndpointI e = factory.read(bs);
                bs.endReadEncaps();
                return e;
            }
            return ue; // Endpoint is opaque, but we don't have a factory for its type.
        }

        return null;
    }

    public synchronized EndpointI read(BasicStream s)
    {
        short type = s.readShort();

        EndpointFactory factory = get(type);
        EndpointI e = null;

        s.startReadEncaps();

        if(factory != null)
        {
            e = factory.read(s);
        }
        else
        {
            e = new OpaqueEndpointI(type, s);
        }

        s.endReadEncaps();

        return e;
    }

    void destroy()
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
