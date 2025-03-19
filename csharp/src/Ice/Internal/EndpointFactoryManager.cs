// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.Internal;

public sealed class EndpointFactoryManager
{
    internal EndpointFactoryManager(Instance instance)
    {
        _instance = instance;
        _factories = new List<EndpointFactory>();
    }

    public void initialize()
    {
        foreach (EndpointFactory f in _factories)
        {
            f.initialize();
        }
    }

    public void add(EndpointFactory factory)
    {
        lock (_mutex)
        {
            Debug.Assert(!_factories.Any(f => f.type() == factory.type()));
            _factories.Add(factory);
        }
    }

    public EndpointFactory get(short type)
    {
        lock (_mutex)
        {
            foreach (EndpointFactory f in _factories)
            {
                if (f.type() == type)
                {
                    return f;
                }
            }
            return null;
        }
    }

    public EndpointI create(string str, bool oaEndpoint)
    {
        string[] arr = Ice.UtilInternal.StringUtil.splitString(str, " \t\r\n") ?? throw new ParseException($"Failed to parse endpoint '{str}': mismatched quote");
        if (arr.Length == 0)
        {
            throw new ParseException($"Failed to parse endpoint '{str}': value has no non-whitespace characters");
        }

        List<string> v = new List<string>(arr);
        string protocol = v[0];
        v.RemoveAt(0);

        if (protocol == "default")
        {
            protocol = _instance.defaultsAndOverrides().defaultProtocol;
        }

        EndpointFactory factory = null;

        lock (_mutex)
        {
            for (int i = 0; i < _factories.Count; i++)
            {
                EndpointFactory f = _factories[i];
                if (f.protocol().Equals(protocol, StringComparison.Ordinal))
                {
                    factory = f;
                }
            }
        }

        if (factory != null)
        {
            EndpointI e = factory.create(v, oaEndpoint);
            if (v.Count > 0)
            {
                throw new ParseException($"Failed to parse endpoint '{str}': unrecognized argument '{v[0]}'");
            }
            return e;

            // Code below left in place for debugging.

            /*
            EndpointI e = f.create(s.Substring(m.Index + m.Length), oaEndpoint);
            BasicStream bs = new BasicStream(_instance, true);
            e.streamWrite(bs);
            Buffer buf = bs.getBuffer();
            buf.b.position(0);
            short type = bs.readShort();
            EndpointI ue = new Ice.Internal.OpaqueEndpointI(type, bs);
            System.Console.Error.WriteLine("Normal: " + e);
            System.Console.Error.WriteLine("Opaque: " + ue);
            return e;
            */
        }

        //
        // If the stringified endpoint is opaque, create an unknown endpoint,
        // then see whether the type matches one of the known endpoints.
        //
        if (protocol == "opaque")
        {
            EndpointI ue = new OpaqueEndpointI(v);
            if (v.Count > 0)
            {
                throw new ParseException($"Failed to parse endpoint '{str}': unrecognized argument '{v[0]}'");
            }
            factory = get(ue.type());
            if (factory != null)
            {
                //
                // Make a temporary stream, write the opaque endpoint data into the stream,
                // and ask the factory to read the endpoint data from that stream to create
                // the actual endpoint.
                //
                var os = new Ice.OutputStream(Ice.Util.currentProtocolEncoding);
                os.writeShort(ue.type());
                ue.streamWrite(os);
                Ice.InputStream iss =
                    new Ice.InputStream(_instance, Ice.Util.currentProtocolEncoding, os.getBuffer(), true);
                iss.pos(0);
                iss.readShort(); // type
                iss.startEncapsulation();
                EndpointI e = factory.read(iss);
                iss.endEncapsulation();
                return e;
            }
            return ue; // Endpoint is opaque, but we don't have a factory for its type.
        }

        return null;
    }

    public EndpointI read(Ice.InputStream s)
    {
        lock (_mutex)
        {
            short type = s.readShort();

            EndpointFactory factory = get(type);
            EndpointI e = null;

            s.startEncapsulation();

            if (factory != null)
            {
                e = factory.read(s);
            }
            //
            // If the factory failed to read the endpoint, return an opaque endpoint. This can
            // occur if for example the factory delegates to another factory and this factory
            // isn't available. In this case, the factory needs to make sure the stream position
            // is preserved for reading the opaque endpoint.
            //
            e ??= new OpaqueEndpointI(type, s);

            s.endEncapsulation();

            return e;
        }
    }

    private readonly Instance _instance;
    private readonly List<EndpointFactory> _factories;
    private readonly object _mutex = new();
}
