//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

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

        public Endpoint? CreateEndpoint(string endpointString, bool oaEndpoint)
        {
            string[]? args = IceUtilInternal.StringUtil.SplitString(endpointString, " \t\r\n");
            if (args == null)
            {
                throw new FormatException($"mismatched quote in endpoint `{endpointString}'");
            }

            if (args.Length == 0)
            {
                throw new FormatException("no non-whitespace character in endpoint string");
            }

            string transport = args[0];
            if (transport == "default")
            {
                transport = DefaultTransport;
            }

            var options = new Dictionary<string, string?>();

            // Parse args into options (and skip transport at args[0])
            for (int n = 1; n < args.Length; ++n)
            {
                // Any option with < 2 characters or that does not start with - is illegal
                string option = args[n];
                if (option.Length < 2 || option[0] != '-')
                {
                    throw new FormatException($"invalid option `{option}' in endpoint `{endpointString}'");
                }

                // Extract the argument given to the current option, if any
                string? argument = null;
                if (n + 1 < args.Length && args[n + 1][0] != '-')
                {
                    argument = args[++n];
                }

                try
                {
                    options.Add(option, argument);
                }
                catch (ArgumentException)
                {
                    throw new FormatException($"duplicate option `{option}' in endpoint `{endpointString}'");
                }
            }

            IEndpointFactory? factory = null;

            lock (this)
            {
                for (int i = 0; i < _endpointFactories.Count; i++)
                {
                    IEndpointFactory f = _endpointFactories[i];
                    if (f.Transport() == transport)
                    {
                        factory = f;
                    }
                }
            }

            if (factory != null)
            {
                Endpoint? e = factory.Create(endpointString, options, oaEndpoint);
                if (options.Count > 0)
                {
                    throw new FormatException(
                        $"unrecognized option(s) `{ToString(options)}' in endpoint `{endpointString}'");
                }
                return e;
            }

            //
            // If the stringified endpoint is opaque, create an unknown endpoint,
            // then see whether the type matches one of the known endpoints.
            //
            if (transport == "opaque")
            {
                Endpoint ue = new OpaqueEndpoint(endpointString, options);
                if (options.Count > 0)
                {
                    throw new FormatException(
                        $"unrecognized option(s) `{ToString(options)}' in endpoint `{endpointString}'");
                }
                factory = GetEndpointFactory(ue.Type);
                if (factory != null)
                {
                    //
                    // Make a temporary stream, write the opaque endpoint data into the stream,
                    // and ask the factory to read the endpoint data from that stream to create
                    // the actual endpoint.
                    //
                    var ostr = new OutputStream(Ice1Definitions.Encoding, new List<ArraySegment<byte>>());
                    ostr.WriteShort((short)ue.Type);
                    ue.IceWrite(ostr);
                    // TODO avoid copy OutputStream buffers
                    var iss = new InputStream(this, ostr.ToArray());
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

        public IEndpointFactory? GetEndpointFactory(EndpointType type)
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

        public Endpoint ReadEndpoint(InputStream istr)
        {
            lock (this)
            {
                var type = (EndpointType)istr.ReadShort();

                IEndpointFactory? factory = GetEndpointFactory(type);
                Endpoint? e = null;

                (Encoding encoding, int size) = istr.StartEndpointEncapsulation();
                if (factory != null)
                {
                    e = factory.Read(istr);
                }

                // If the factory failed to read the endpoint, return an opaque endpoint. This can
                // occur if for example the factory delegates to another factory and this factory
                // isn't available. In this case, the factory needs to make sure the stream position
                // is preserved for reading the opaque endpoint.
                if (e == null)
                {
                    byte[] data = new byte[size];
                    size = istr.ReadSpan(data);
                    if (size < data.Length)
                    {
                        throw new InvalidDataException(@$"not enough bytes available reading opaque endpoint, requested {
                            data.Length} bytes, but there was only {size} bytes remaining");
                    }
                    e = new OpaqueEndpoint(type, encoding, data);
                }
                istr.EndEndpointEncapsulation();

                return e;
            }
        }

        private static string ToString(Dictionary<string, string?> options)
        {
            StringBuilder sb = new StringBuilder();
            foreach ((string option, string? argument) in options)
            {
                if (sb.Length > 0)
                {
                    sb.Append(" ");
                }
                sb.Append(option);
                if (argument != null)
                {
                    sb.Append(" ");
                    sb.Append(argument);
                }
            }
            return sb.ToString();
        }
    }
}
