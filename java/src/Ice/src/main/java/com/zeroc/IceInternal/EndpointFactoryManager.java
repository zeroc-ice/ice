// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public final class EndpointFactoryManager
{
    EndpointFactoryManager(Instance instance)
    {
        _instance = instance;
    }

    public void initialize()
    {
        for(EndpointFactory f : _factories)
        {
            f.initialize();
        }
    }

    public synchronized void add(EndpointFactory factory)
    {
        for(EndpointFactory f : _factories)
        {
            if(f.type() == factory.type())
            {
                assert(false);
            }
        }
        _factories.add(factory);
    }

    public synchronized EndpointFactory get(short type)
    {
        for(EndpointFactory f : _factories)
        {
            if(f.type() == type)
            {
                return f;
            }
        }
        return null;
    }

    public synchronized EndpointI create(String str, boolean oaEndpoint)
    {
        String[] arr = com.zeroc.IceUtilInternal.StringUtil.splitString(str, " \t\r\n");
        if(arr == null)
        {
            com.zeroc.Ice.EndpointParseException e = new com.zeroc.Ice.EndpointParseException();
            e.str = "mismatched quote";
            throw e;
        }

        if(arr.length == 0)
        {
            com.zeroc.Ice.EndpointParseException e = new com.zeroc.Ice.EndpointParseException();
            e.str = "value has no non-whitespace characters";
            throw e;
        }

        java.util.ArrayList<String> v = new java.util.ArrayList<>(java.util.Arrays.asList(arr));
        String protocol = v.get(0);
        v.remove(0);

        if(protocol.equals("default"))
        {
            protocol = _instance.defaultsAndOverrides().defaultProtocol;
        }

        EndpointFactory factory = null;

        for(EndpointFactory f : _factories)
        {
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
                com.zeroc.Ice.EndpointParseException ex = new com.zeroc.Ice.EndpointParseException();
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
            EndpointI ue = new OpaqueEndpointI(type, bs);
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
                com.zeroc.Ice.EndpointParseException ex = new com.zeroc.Ice.EndpointParseException();
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
                com.zeroc.Ice.OutputStream os =
                    new com.zeroc.Ice.OutputStream(_instance, Protocol.currentProtocolEncoding, false);
                os.writeShort(ue.type());
                ue.streamWrite(os);
                com.zeroc.Ice.InputStream is =
                    new com.zeroc.Ice.InputStream(_instance, Protocol.currentProtocolEncoding, os.getBuffer(), true);
                is.pos(0);
                is.readShort(); // type
                is.startEncapsulation();
                EndpointI e = factory.read(is);
                is.endEncapsulation();
                return e;
            }
            return ue; // Endpoint is opaque, but we don't have a factory for its type.
        }

        return null;
    }

    public synchronized EndpointI read(com.zeroc.Ice.InputStream s)
    {
        short type = s.readShort();

        EndpointFactory factory = get(type);
        EndpointI e = null;

        s.startEncapsulation();

        if(factory != null)
        {
            e = factory.read(s);
        }
        //
        // If the factory failed to read the endpoint, return an opaque endpoint. This can
        // occur if for example the factory delegates to another factory and this factory
        // isn't available. In this case, the factory needs to make sure the stream position
        // is preserved for reading the opaque endpoint.
        //
        if(e == null)
        {
            e = new OpaqueEndpointI(type, s);
        }

        s.endEncapsulation();

        return e;
    }

    void destroy()
    {
        for(EndpointFactory f : _factories)
        {
            f.destroy();
        }
        _factories.clear();
    }

    private Instance _instance;
    private java.util.List<EndpointFactory> _factories = new java.util.ArrayList<>();
}
