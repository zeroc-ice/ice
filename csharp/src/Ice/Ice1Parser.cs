//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Text;

namespace ZeroC.Ice
{

    /// <summary>Provides helper methods to parse proxy and endpoint strings in the ice1 format.</summary>
    internal static class Ice1Parser
    {
        /// <summary>Creates an endpoint from a string in the ice1 format.</summary>
        /// <param name="endpointString">The string parsed by this method.</param>
        /// <param name="communicator">The communicator of the enclosing proxy or object adapter.</param>
        /// <param name="oaEndpoint">When true, endpointString represents an object adapter's endpoint configuration;
        /// when false, endpointString represents a proxy endpoint.</param>
        /// <returns>The new endpoint.</returns>
        /// <exception cref="FormatException">Thrown when endpointString cannot be parsed.</exception>
        /// <exception cref="NotSupportedException">Thrown when the transport specified in endpointString does not
        /// the ice1 protocol.</exception>
        internal static Endpoint ParseEndpoint(string endpointString, Communicator communicator, bool oaEndpoint)
        {
            string[]? args = StringUtil.SplitString(endpointString, " \t\r\n");
            if (args == null)
            {
                throw new FormatException($"mismatched quote in endpoint `{endpointString}'");
            }

            if (args.Length == 0)
            {
                throw new FormatException("no non-whitespace character in endpoint string");
            }

            string transportName = args[0];
            if (transportName == "default")
            {
                transportName = communicator.DefaultTransport;
            }

            var options = new Dictionary<string, string?>();

            // Parse args into options (and skip transportName at args[0])
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

            if (communicator.FindEndpointFactory(transportName) is (IEndpointFactory factory, Transport transport))
            {
                Endpoint endpoint = factory.Create(transport, Protocol.Ice1, options, oaEndpoint, endpointString);
                if (options.Count > 0)
                {
                    throw new FormatException(
                        $"unrecognized option(s) `{ToString(options)}' in endpoint `{endpointString}'");
                }
                return endpoint;
            }

            // If the stringified endpoint is opaque, create an unknown endpoint, then see whether the type matches one
            // of the known endpoints.
            if (!oaEndpoint && transportName == "opaque")
            {
                var opaqueEndpoint = new OpaqueEndpoint(communicator, options, endpointString);
                if (options.Count > 0)
                {
                    throw new FormatException(
                        $"unrecognized option(s) `{ToString(options)}' in endpoint `{endpointString}'");
                }

                if (opaqueEndpoint.ValueEncoding.IsSupported &&
                    communicator.IceFindEndpointFactory(opaqueEndpoint.Transport) != null)
                {
                    // We may be able to unmarshal this endpoint, so we first marshal it into a byte buffer and then
                    // unmarshal it from this buffer.
                    var bufferList = new List<ArraySegment<byte>>
                    {
                        // 8 = size of short + size of encapsulation header with 1.1 encoding
                        new byte[8 + opaqueEndpoint.Value.Length]
                    };

                    var ostr = new OutputStream(Ice1Definitions.Encoding, bufferList);
                    ostr.WriteEndpoint(opaqueEndpoint);
                    OutputStream.Position tail = ostr.Save();
                    Debug.Assert(bufferList.Count == 1);
                    Debug.Assert(tail.Segment == 0 && tail.Offset == 8 + opaqueEndpoint.Value.Length);

                    return new InputStream(Ice1Definitions.Encoding, bufferList[0]).ReadEndpoint(Protocol.Ice1,
                                                                                                 communicator);
                }
                else
                {
                    return opaqueEndpoint;
                }
            }

            throw new FormatException($"unknown transport `{transportName}' in endpoint `{endpointString}'");
        }

        // Stringify the options of an endpoint
        private static string ToString(Dictionary<string, string?> options)
        {
            var sb = new StringBuilder();
            foreach ((string option, string? argument) in options)
            {
                if (sb.Length > 0)
                {
                    sb.Append(' ');
                }
                sb.Append(option);
                if (argument != null)
                {
                    sb.Append(' ');
                    sb.Append(argument);
                }
            }
            return sb.ToString();
        }
    }
}
