//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
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
                         List<string> Path,
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
                    throw new FormatException("an object adapter endpoint supports only ice+transport URIs");
                }

                var generalOptions = new Dictionary<string, string>();
                Dictionary<string, string>? endpointOptions = iceScheme ? null : new Dictionary<string, string>();

                Uri uri = InitialParse(uriString, generalOptions, endpointOptions);

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
                        throw new FormatException("option `invocation-mode' requires the ice1 protocol");
                    }
                    if (oaEndpoints)
                    {
                        throw new FormatException(
                            "option `invocation-mode' is not applicable to object adapter endpoints");
                    }
                    if (int.TryParse(invocationModeValue, out int _))
                    {
                        throw new FormatException($"invalid value `{invocationModeValue}' for invocation-mode");
                    }
                    invocationMode = Enum.Parse<InvocationMode>(invocationModeValue, ignoreCase: true);
                }

                string facet = uri.Fragment.Length >= 2 ? Uri.UnescapeDataString(uri.Fragment.TrimStart('#')) : "";
                List<string> path =
                    uri.AbsolutePath.TrimStart('/').Split('/').Select(s => Uri.UnescapeDataString(s)).ToList();

                List<Endpoint>? endpoints = null;

                if (endpointOptions != null) // i.e. not ice scheme
                {
                    endpoints = new List<Endpoint>
                    {
                        CreateEndpoint(communicator,
                                       oaEndpoints,
                                       endpointOptions,
                                       protocol,
                                       uri,
                                       uriString)
                    };

                    if (generalOptions.TryGetValue("alt-endpoint", out string? altEndpointValue))
                    {
                        foreach (string endpointStr in altEndpointValue.Split(','))
                        {
                            if (endpointStr.StartsWith("ice:"))
                            {
                                throw new FormatException(
                                    $"invalid URI scheme for endpoint `{endpointStr}': must be empty or ice+transport");
                            }

                            string altUriString = endpointStr;
                            if (!altUriString.StartsWith("ice+"))
                            {
                                altUriString = $"{uri.Scheme}://{altUriString}";
                            }

                            // The separator for endpoint options in alt-endpoint is $, and we replace these $ by &
                            // before sending the string the main parser (InitialParse), which uses & as separator.
                            altUriString = altUriString.Replace('$', '&');

                            // No need to clear endpointOptions before reusing it since CreateEndpoint consumes all the
                            // endpoint options
                            Debug.Assert(endpointOptions.Count == 0);
                            uri = InitialParse(altUriString, generalOptions: null, endpointOptions);

                            Debug.Assert(uri.AbsolutePath[0] == '/'); // there is always a first segment
                            if (uri.AbsolutePath.Length > 1 || uri.Fragment.Length > 0)
                            {
                                throw new FormatException(
                                    $"endpoint `{endpointStr}' must not specify a path or fragment");
                            }
                            endpoints.Add(CreateEndpoint(communicator,
                                                         oaEndpoints,
                                                         endpointOptions,
                                                         protocol,
                                                         uri,
                                                         endpointStr));
                        }
                    }
                }

                return (encoding,
                        (IReadOnlyList<Endpoint>?)endpoints ?? ImmutableArray<Endpoint>.Empty,
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
            RegisterTransport("universal", defaultPort: 0);

            // There is actually no authority at all with the ice scheme, but we emulate it with an empty authority
            // during parsing by the Uri class and the GenericUriParser.
            GenericUriParserOptions options =
                ParserOptions |
                GenericUriParserOptions.AllowEmptyAuthority |
                GenericUriParserOptions.NoPort;

            System.UriParser.Register(new GenericUriParser(options), "ice", -1);
        }

        /// <summary>Registers an ice+transport scheme.</summary>
        /// <param name="transportName">The name of the transport (cannot be empty).</param>
        /// <param name="defaultPort">The default port for this transport.</param>
        internal static void RegisterTransport(string transportName, ushort defaultPort) =>
            System.UriParser.Register(new GenericUriParser(ParserOptions), $"ice+{transportName}", defaultPort);

        private static Endpoint CreateEndpoint(
            Communicator communicator,
            bool oaEndpoint,
            Dictionary<string, string> options,
            Protocol protocol,
            Uri uri,
            string uriString)
        {
            Debug.Assert(uri.Scheme.StartsWith("ice+"));
            string transportName = uri.Scheme.Substring(4); // i.e. chop-off "ice+"

            ushort port = 0;
            checked
            {
                port = (ushort)uri.Port;
            }

            IEndpointFactory? factory = null;
            Transport transport = default;

            if (transportName == "universal")
            {
                if (protocol == Protocol.Ice1)
                {
                    throw new FormatException("ice+universal is not compatible with the ice1 protocol");
                }
                if (oaEndpoint)
                {
                    throw new FormatException("ice+universal cannot specify an object adapter endpoint");
                }

                // Enumerator names can only be used for "well-known" transports.
                transport = Enum.Parse<Transport>(options["transport"], ignoreCase: true);
                options.Remove("transport");

                // It's possible we have a factory for this transport:
                factory = communicator.IceFindEndpointFactory(transport);
            }
            else if (communicator.FindEndpointFactory(transportName) is (EndpointFactory f, Transport t))
            {
                factory = f;
                transport = t;
            }
            else
            {
                throw new FormatException($"unknown transport `{transportName}'");
            }

            Endpoint endpoint = factory?.Create(transport,
                                                protocol,
                                                uri.DnsSafeHost,
                                                port,
                                                options,
                                                oaEndpoint,
                                                uriString) ??
                new OpaqueEndpoint(communicator, transport, protocol, uri.DnsSafeHost, port, options);

            if (options.Count > 0)
            {
                throw new FormatException($"unknown option `{options.First().Key}' for transport `{transportName}'");
            }
            return endpoint;
        }

        private static Uri InitialParse(
            string uriString,
            Dictionary<string, string>? generalOptions,
            Dictionary<string, string>? endpointOptions)
        {
            if (endpointOptions == null) // i.e. ice scheme
            {
                Debug.Assert(uriString.StartsWith("ice:"));
                Debug.Assert(generalOptions != null);

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

            var uri = new Uri(uriString);

            string[] nvPairs = uri.Query.Length >= 2 ? uri.Query.TrimStart('?').Split('&') : Array.Empty<string>();

            foreach (string p in nvPairs)
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
                    AppendValue(generalOptions, name, value, uriString);
                }
                else if (endpointOptions == null)
                {
                    throw new FormatException($"the ice URI scheme does not support option `{name}'");
                }
                else if (name == "alt-endpoint")
                {
                    AppendValue(generalOptions, name, value, uriString);
                }
                else
                {
                    AppendValue(endpointOptions, name, value, uriString);
                }
            }

            return uri;

            static void AppendValue(Dictionary<string, string>? options, string name, string value, string uriString)
            {
                if (options == null)
                {
                    throw new FormatException($"unexpected option `{name}' in endpoint `{uriString}'");
                }

                if (options.TryGetValue(name, out string? existingValue))
                {
                    options[name] = $"{existingValue},{value}";
                }
                else
                {
                    options.Add(name, value);
                }
            }
        }
    }
}
