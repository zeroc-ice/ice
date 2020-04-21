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
                Endpoint endpoint = factory.Create(endpointString, options, oaEndpoint);
                if (options.Count > 0)
                {
                    throw new FormatException(
                        $"unrecognized option(s) `{ToString(options)}' in endpoint `{endpointString}'");
                }
                return endpoint;
            }

            //
            // If the stringified endpoint is opaque, create an unknown endpoint,
            // then see whether the type matches one of the known endpoints.
            //
            if (transport == "opaque")
            {
                OpaqueEndpoint opaqueEndpoint = new OpaqueEndpoint(endpointString, options);
                if (options.Count > 0)
                {
                    throw new FormatException(
                        $"unrecognized option(s) `{ToString(options)}' in endpoint `{endpointString}'");
                }

                if (opaqueEndpoint.Encoding.IsSupported && GetEndpointFactory(opaqueEndpoint.Type) != null)
                {
                    // We may be able to unmarshal this endpoint, so we first marshal it into a byte buffer and then
                    // unmarshal it from this buffer.
                    var bufferList = new List<ArraySegment<byte>>();
                    // 8 = size of short + size of encapsulation header
                    bufferList.Add(new byte[8 + opaqueEndpoint.Bytes.Length]);
                    var ostr = new OutputStream(Ice1Definitions.Encoding, bufferList);
                    ostr.WriteEndpoint(opaqueEndpoint);
                    OutputStream.Position tail = ostr.Save();
                    Debug.Assert(bufferList.Count == 1);
                    Debug.Assert(tail.Segment == 0 && tail.Offset == 8 + opaqueEndpoint.Bytes.Length);

                    return new InputStream(this, bufferList[0]).ReadEndpoint();
                }
                else
                {
                    return opaqueEndpoint;
                }
            }

            return null;
        }

        internal IEndpointFactory? GetEndpointFactory(EndpointType type)
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
