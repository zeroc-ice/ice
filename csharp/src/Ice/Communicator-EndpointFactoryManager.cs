//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    public sealed partial class Communicator
    {
        private readonly List<IEndpointFactory> _endpointFactories;

        public void AddEndpointFactory(IEndpointFactory factory)
        {
            lock (this)
            {
                foreach (IEndpointFactory f in _endpointFactories)
                {
                    if (f.Type() == factory.Type())
                    {
                        Debug.Assert(false);
                    }
                }
                _endpointFactories.Add(factory);
            }
        }

        public Endpoint? CreateEndpoint(string str, bool oaEndpoint)
        {
            string[]? arr = IceUtilInternal.StringUtil.SplitString(str, " \t\r\n");
            if (arr == null)
            {
                throw new System.FormatException("mismatched quote");
            }

            if (arr.Length == 0)
            {
                throw new System.FormatException("value has no non-whitespace characters");
            }

            var v = new List<string>(arr);
            string transport = v[0];
            v.RemoveAt(0);

            if (transport.Equals("default"))
            {
                transport = DefaultsAndOverrides.DefaultTransport;
            }

            IEndpointFactory? factory = null;

            lock (this)
            {
                for (int i = 0; i < _endpointFactories.Count; i++)
                {
                    IEndpointFactory f = _endpointFactories[i];
                    if (f.Transport().Equals(transport))
                    {
                        factory = f;
                    }
                }
            }

            if (factory != null)
            {
                Endpoint? e = factory.Create(v, oaEndpoint);
                if (v.Count > 0)
                {
                    throw new System.FormatException($"unrecognized argument `{v[0]}' in endpoint `{str}'");
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
            if (transport.Equals("opaque"))
            {
                Endpoint ue = new OpaqueEndpointI(v);
                if (v.Count > 0)
                {
                    throw new System.FormatException($"unrecognized argument `{v[0]}' in endpoint `{str}'");
                }
                factory = GetEndpointFactory(ue.Type());
                if (factory != null)
                {
                    //
                    // Make a temporary stream, write the opaque endpoint data into the stream,
                    // and ask the factory to read the endpoint data from that stream to create
                    // the actual endpoint.
                    //
                    var os = new OutputStream(Ice1Definitions.Encoding, new List<ArraySegment<byte>>());
                    os.WriteShort(ue.Type());
                    ue.StreamWrite(os);
                    // TODO avoid copy OutputStream buffers
                    var iss = new InputStream(this, Ice1Definitions.Encoding, os.ToArray());
                    iss.Pos = 0;
                    iss.ReadShort(); // type
                    iss.StartEndpointEncapsulation();
                    Endpoint? e = factory.Read(iss);
                    iss.EndEndpointEncapsulation();
                    return e;
                }
                return ue; // Endpoint is opaque, but we don't have a factory for its type.
            }

            return null;
        }

        public IEndpointFactory? GetEndpointFactory(short type)
        {
            lock (this)
            {
                foreach (IEndpointFactory f in _endpointFactories)
                {
                    if (f.Type() == type)
                    {
                        return f;
                    }
                }
                return null;
            }
        }

        public Endpoint ReadEndpoint(InputStream s)
        {
            lock (this)
            {
                short type = s.ReadShort();

                IEndpointFactory? factory = GetEndpointFactory(type);
                Endpoint? e = null;

                s.StartEndpointEncapsulation();

                if (factory != null)
                {
                    e = factory.Read(s);
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

                s.EndEndpointEncapsulation();

                return e;
            }
        }
    }

}
