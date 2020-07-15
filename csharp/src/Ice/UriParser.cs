//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

namespace ZeroC.Ice
{

    /// <summary>Provides helper methods to parse and print URI strings that comply with the ice and ice+transport
    /// URI schemes.</summary>
    internal static class UriParser
    {
        // Common options for the ice and ice[+transport] parsers we register for each transport.
        private const GenericUriParserOptions ParserOptions =
            GenericUriParserOptions.DontConvertPathBackslashes |
            GenericUriParserOptions.DontUnescapePathDotsAndSlashes |
            GenericUriParserOptions.Idn |
            GenericUriParserOptions.IriParsing |
            GenericUriParserOptions.NoUserInfo;

        /// <summary>Checks if a string is an ice or ice+transport URI and not a stringified proxy using the old format.
        /// </summary>
        /// <param name="s">The string to check.</param>
        /// <returns>True when the string is most likely an ice or ice+transport URI; otherwise, false.</returns>
        internal static bool IsUri(string s) =>
            (s.StartsWith("ice:") || (s.StartsWith("ice+") && s.Contains(':'))) && !s.Contains(' ');

        /// <summary>Parses an ice or ice+transport URI string.</summary>
        /// <param name="uriString">The URI string to parse.</param>
        /// <param name="oaEndpoints">When true, the URI is used to configure an object adapter.</param>
        /// <param name="communicator">The communicator.</param>
        /// <returns>The parsed components of the ice or ice+transport URI.</returns>
        internal static (Encoding Encoding,
                         IReadOnlyList<Endpoint> Endpoints,
                         string Facet,
                         InvocationMode InvocationMode,
                         IReadOnlyList<string> Path,
                         Protocol Protocol) Parse(
            string uriString,
            bool oaEndpoints,
            Communicator communicator)
        {
            Debug.Assert(IsUri(uriString));

            try
            {
                bool iceScheme = uriString.StartsWith("ice:");
                if (iceScheme && oaEndpoints)
                {
                    throw new FormatException("an object adapter configuration supports only ice+transport URIs");
                }

                var generalOptions = new Dictionary<string, string>();
                Dictionary<string, string>? endpointOptions = iceScheme ? null : new Dictionary<string, string>();

                (Uri uri, string transportName) = InitialParse(uriString, generalOptions, endpointOptions);

                Protocol protocol = Protocol.Ice2;
                if (generalOptions.TryGetValue("protocol", out string? protocolValue))
                {
                    protocol = ProtocolExtensions.Parse(protocolValue);
                }

                Encoding encoding = protocol.IsSupported() ? protocol.GetEncoding() : Encoding.V2_0;
                if (generalOptions.TryGetValue("encoding", out string? encodingValue))
                {
                    encoding = Encoding.Parse(encodingValue);
                }

                InvocationMode invocationMode = InvocationMode.Twoway;
                if (generalOptions.TryGetValue("invocation-mode", out string? invocationModeValue))
                {
                    if (protocol != Protocol.Ice1)
                    {
                        throw new FormatException("the option invocation-mode applies only to the ice1 protocol");
                    }
                    if (oaEndpoints)
                    {
                        throw new FormatException("the option invocation-mode applies only to proxies");
                    }
                    if (int.TryParse(invocationModeValue, out int _))
                    {
                        throw new FormatException("the option invocation-mode does not accept numeric values");
                    }
                    invocationMode = Enum.Parse<InvocationMode>(invocationModeValue, ignoreCase: true);
                }

                string facet = uri.Fragment.Length >= 2 ? Uri.UnescapeDataString(uri.Fragment.Substring(1)) : "";
                List<string> path = uri.AbsolutePath.Split('/').Select(s => Uri.UnescapeDataString(s)).ToList();

                Debug.Assert(path.Count > 0); // there is always a first empty segment that we drop
                Debug.Assert(path[0].Length == 0);
                path.RemoveAt(0);

                List<Endpoint>? endpoints = null;

                if (endpointOptions != null) // i.e. not ice scheme
                {
                    endpoints = new List<Endpoint>
                    {
                        CreateEndpoint(communicator,
                                       oaEndpoints,
                                       endpointOptions,
                                       protocol,
                                       transportName,
                                       uri,
                                       uriString)
                    };

                    if (generalOptions.TryGetValue("alt-endpoints", out string? altEndpointsValue))
                    {
                        string[] altEndpoints = altEndpointsValue.Split(',');
                        foreach (string alt in altEndpoints)
                        {
                            if (alt.StartsWith("ice:"))
                            {
                                throw new FormatException(
                                    $"invalid scheme for alt-endpoint `{alt}': must be empty or ice+transport");
                            }

                            string altUriString = alt;
                            if (!altUriString.StartsWith("ice+"))
                            {
                                altUriString = $"{uri.Scheme}://{altUriString}";
                            }
                            altUriString = altUriString.Replace('$', '&');

                            // No need to clear endpointOptions before reusing it since CreateEndpoint consumes all the
                            // endpoint options
                            Debug.Assert(endpointOptions.Count == 0);
                            (uri, transportName) = InitialParse(altUriString, generalOptions: null, endpointOptions);

                            // > 1 because there is always a first empty segment.
                            if (uri.AbsolutePath.Length > 1 || uri.Fragment.Length > 0)
                            {
                                throw new FormatException($"alt-endpoint `{alt}' must not specify a path or fragment");
                            }
                            endpoints.Add(CreateEndpoint(communicator,
                                                         oaEndpoints,
                                                         endpointOptions,
                                                         protocol,
                                                         transportName,
                                                         uri,
                                                         alt));
                        }
                    }
                }

                return (encoding,
                        (IReadOnlyList<Endpoint>?)endpoints ?? (IReadOnlyList<Endpoint>)Array.Empty<Endpoint>(),
                        facet,
                        invocationMode,
                        path,
                        protocol);
            }
            catch (Exception ex)
            {
                // Give context to the exception.
                throw new FormatException($"failed to parse URI `{uriString}'", ex);
            }
        }

        /// <summary>Registers the ice and ice+universal schemes.</summary>
        internal static void RegisterCommon()
        {
            RegisterTransport("universal", defaultPort: 0, ipHost: false);

            // There is no authority at all with the ice scheme
            GenericUriParserOptions options =
                ParserOptions |
                GenericUriParserOptions.AllowEmptyAuthority |
                GenericUriParserOptions.NoPort;

            System.UriParser.Register(new GenericUriParser(options), "ice", -1);
        }

        /// <summary>Registers an ice+transport scheme.</summary>
        /// <param name="transportName">The name of the transport (cannot be empty).</param>
        /// <param name="defaultPort">The default port for this transport.</param>
        /// <param name="ipHost">When true, the host must be an IPv4 or an IPv6 address, or a valid
        /// DNS name (but no DNS lookup is performed).</param>
        internal static void RegisterTransport(string transportName, ushort defaultPort, bool ipHost)
        {
            Debug.Assert(transportName.Length > 0);
            GenericUriParserOptions options = ParserOptions;
            if (!ipHost)
            {
                options |= GenericUriParserOptions.GenericAuthority;
            }
            System.UriParser.Register(new GenericUriParser(options), $"ice+{transportName}", defaultPort);
        }

        private static Endpoint CreateEndpoint(
            Communicator communicator,
            bool oaEndpoint,
            Dictionary<string, string> options,
            Protocol protocol,
            string transportName,
            Uri uri,
            string uriString)
        {
            ushort port = 0;
            checked
            {
                port = (ushort)uri.Port;
            }

            Endpoint result;
            if (transportName == "universal")
            {
                if (protocol == Protocol.Ice1)
                {
                    throw new FormatException("the ice+universal scheme is not compatible with the ice1 protocol");
                }

                // The transport name / number should never be escaped. Enumerator names can only be used for
                // "well-known" transports.
                Transport transport = Enum.Parse<Transport>(options["transport"], ignoreCase: true);
                options.Remove("transport");
                result = new OpaqueEndpoint(communicator, transport, protocol, uri.Host, port, options);
            }
            else if (communicator.FindEndpointFactory(transportName) is (EndpointFactory factory, Transport transport))
            {
                result = factory.Create(transport,
                                        protocol,
                                        uri.Host, // can be an IPv6 address in brackets
                                        port,
                                        options,
                                        oaEndpoint,
                                        uriString);
            }
            else
            {
                throw new FormatException($"unknown transport `{transportName}'");
            }

            if (options.Count > 0)
            {
                throw new FormatException($"unknown option `{options.First().Key}' for transport `{transportName}'");
            }
            return result;
        }

        private static (Uri Uri, string TransportName) InitialParse(
            string uriString,
            Dictionary<string, string>? generalOptions,
            Dictionary<string, string>? endpointOptions = null)
        {
            if (uriString.StartsWith("ice:"))
            {
                Debug.Assert(endpointOptions == null);

                string body = uriString.Substring(4);
                if (body.StartsWith("//"))
                {
                    throw new FormatException("the ice URI scheme cannot define a host or port");
                }
                // Add empty authority for Uri's constructor.
                if (body.StartsWith('/'))
                {
                    uriString = $"ice://{body}";
                }
                else
                {
                    uriString = $"ice:///{body}";
                }
            }
            else
            {
                Debug.Assert(endpointOptions != null);
            }

            var uri = new Uri(uriString);

            string transportName = uri.Scheme.Length >= 4 ? uri.Scheme.Substring(4) : "";

            string[] nvPairArray;
            if (uri.Query.Length >= 2)
            {
                nvPairArray = uri.Query.Substring(1).Split('&');
            }
            else
            {
                nvPairArray = Array.Empty<string>();
            }

            foreach (string p in nvPairArray)
            {
                int equalPos = p.IndexOf('=');
                if (equalPos <= 0 || equalPos == p.Length - 1)
                {
                    throw new FormatException($"invalid option `{p}'");
                }
                string name = p.Substring(0, equalPos);
                string value = p.Substring(equalPos + 1);

                if (name == "protocol" || name == "encoding" || name == "invocation-mode")
                {
                    if (generalOptions == null)
                    {
                        throw new FormatException($"unexpected option `{name}' in alt-endpoint");
                    }
                    else
                    {
                        generalOptions.Add(name, value);
                    }
                }
                else if (endpointOptions == null)
                {
                    throw new FormatException($"the ice URI scheme does not support option `{name}'");
                }
                else if (name == "alt-endpoints")
                {
                    if (generalOptions == null)
                    {
                        throw new FormatException($"unexpected option `{name}' in alt-endpoint");
                    }
                    else
                    {
                        generalOptions.Add(name, value);
                    }
                }
                else
                {
                    // Note that the options - in particular their values - are still escaped.
                    endpointOptions.Add(name, value);
                }
            }

            return (uri, transportName);
        }
    }
}
