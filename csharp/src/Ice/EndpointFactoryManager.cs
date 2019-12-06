//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System.Collections.Generic;
    using System.Diagnostics;

    public sealed class EndpointFactoryManager
    {
        internal EndpointFactoryManager(Ice.Communicator communicator)
        {
            _communicator = communicator;
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
            lock (this)
            {
                foreach (EndpointFactory f in _factories)
                {
                    if (f.type() == factory.type())
                    {
                        Debug.Assert(false);
                    }
                }
                _factories.Add(factory);
            }
        }

        public EndpointFactory get(short type)
        {
            lock (this)
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
            string[] arr = IceUtilInternal.StringUtil.splitString(str, " \t\r\n");
            if (arr == null)
            {
                Ice.EndpointParseException e = new Ice.EndpointParseException();
                e.str = "mismatched quote";
                throw e;
            }

            if (arr.Length == 0)
            {
                Ice.EndpointParseException e = new Ice.EndpointParseException();
                e.str = "value has no non-whitespace characters";
                throw e;
            }

            List<string> v = new List<string>(arr);
            string protocol = v[0];
            v.RemoveAt(0);

            if (protocol.Equals("default"))
            {
                protocol = _communicator.defaultsAndOverrides().defaultProtocol;
            }

            EndpointFactory factory = null;

            lock (this)
            {
                for (int i = 0; i < _factories.Count; i++)
                {
                    EndpointFactory f = _factories[i];
                    if (f.protocol().Equals(protocol))
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
                    Ice.EndpointParseException ex = new Ice.EndpointParseException();
                    ex.str = "unrecognized argument `" + v[0] + "' in endpoint `" + str + "'";
                    throw ex;
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
                EndpointI ue = new IceInternal.OpaqueEndpointI(type, bs);
                System.Console.Error.WriteLine("Normal: " + e);
                System.Console.Error.WriteLine("Opaque: " + ue);
                return e;
                */
            }

            //
            // If the stringified endpoint is opaque, create an unknown endpoint,
            // then see whether the type matches one of the known endpoints.
            //
            if (protocol.Equals("opaque"))
            {
                EndpointI ue = new OpaqueEndpointI(v);
                if (v.Count > 0)
                {
                    Ice.EndpointParseException ex = new Ice.EndpointParseException();
                    ex.str = "unrecognized argument `" + v[0] + "' in endpoint `" + str + "'";
                    throw ex;
                }
                factory = get(ue.type());
                if (factory != null)
                {
                    //
                    // Make a temporary stream, write the opaque endpoint data into the stream,
                    // and ask the factory to read the endpoint data from that stream to create
                    // the actual endpoint.
                    //
                    Ice.OutputStream os = new Ice.OutputStream(_communicator, Ice.Util.currentProtocolEncoding);
                    os.writeShort(ue.type());
                    ue.streamWrite(os);
                    Ice.InputStream iss =
                        new Ice.InputStream(_communicator, Ice.Util.currentProtocolEncoding, os.getBuffer(), true);
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
            lock (this)
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
                if (e == null)
                {
                    e = new OpaqueEndpointI(type, s);
                }

                s.endEncapsulation();

                return e;
            }
        }

        internal void destroy()
        {
            foreach (EndpointFactory f in _factories)
            {
                f.destroy();
            }
            _factories.Clear();
        }

        private readonly Ice.Communicator _communicator;
        private readonly List<EndpointFactory> _factories;
    }

}
