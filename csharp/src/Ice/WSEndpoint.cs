// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace ZeroC.Ice
{
    internal sealed class WSEndpoint : TcpEndpoint
    {
        public override bool IsAlwaysSecure => Transport == Transport.WSS;

        public override string? this[string option] => option == "resource" ? Resource : base[option];

        /// <summary>A URI specifying the resource associated with this endpoint. The value is passed as the target for
        /// GET in the WebSocket upgrade request.</summary>
        private string Resource => Data.Options.Length > 0 ? Data.Options[0] : "/";

        protected internal override bool HasOptions => Data.Options.Length > 0 || base.HasOptions;

        // There is no Equals or GetHashCode because they are identical to the base.

        // TODO: can we remove this override?
        public override bool IsLocal(Endpoint endpoint) => endpoint is WSEndpoint && base.IsLocal(endpoint);

        protected internal override void WriteOptions(OutputStream ostr)
        {
            Debug.Assert(Protocol == Protocol.Ice1);
            base.WriteOptions(ostr);
            ostr.WriteString(Resource);
        }

        protected internal override void AppendOptions(StringBuilder sb, char optionSeparator)
        {
            base.AppendOptions(sb, optionSeparator);
            if (Resource != "/")
            {
                if (Protocol == Protocol.Ice1)
                {
                    sb.Append(" -r ");
                    bool addQuote = Resource.IndexOf(':') != -1;
                    if (addQuote)
                    {
                        sb.Append('"');
                    }
                    sb.Append(Resource);
                    if (addQuote)
                    {
                        sb.Append('"');
                    }
                }
                else
                {
                    if (base.HasOptions)
                    {
                        sb.Append(optionSeparator);
                    }
                    sb.Append("resource=");
                    // resource must be in a URI-compatible format, with for example spaces escaped as %20.
                    sb.Append(Resource);
                }
            }
        }
        internal static new WSEndpoint CreateIce1Endpoint(Transport transport, InputStream istr)
        {
            Debug.Assert(transport == Transport.WS || transport == Transport.WSS);

            string host = istr.ReadString();
            ushort port = ReadPort(istr);
            var timeout = TimeSpan.FromMilliseconds(istr.ReadInt());
            bool compress = istr.ReadBool();
            string resource = istr.ReadString();

            string[] options = resource == "/" ? Array.Empty<string>() : new string[] { resource };

            return new WSEndpoint(new EndpointData(transport, host, port, options),
                                  timeout,
                                  compress,
                                  istr.Communicator!);
        }

        internal static new WSEndpoint CreateIce2Endpoint(EndpointData data, Communicator communicator)
        {
            Debug.Assert(data.Transport == Transport.WS || data.Transport == Transport.WSS); // TODO: remove WSS

            string[] options = data.Options;
            if (options.Length > 1)
            {
                // Drop extra options since we don't understand them.
                options = new string[] { options[0] };
            }

            return new WSEndpoint(new EndpointData(data.Transport, data.Host, data.Port, options), communicator);
        }

        internal static new WSEndpoint ParseIce1Endpoint(
            Transport transport,
            Dictionary<string, string?> options,
            Communicator communicator,
            bool oaEndpoint,
            string endpointString)
        {
            (string host, ushort port) = ParseHostAndPort(options, oaEndpoint, endpointString);

            string resource = "/";

            if (options.TryGetValue("-r", out string? argument))
            {
                resource = argument ??
                    throw new FormatException($"no argument provided for -r option in endpoint `{endpointString}'");

                options.Remove("-r");
            }

            string[] endpointDataOptions = resource == "/" ? Array.Empty<string>() : new string[] { resource };

            return new WSEndpoint(new EndpointData(transport, host, port, endpointDataOptions),
                                  ParseTimeout(options, endpointString),
                                  ParseCompress(options, endpointString),
                                  options,
                                  communicator,
                                  oaEndpoint,
                                  endpointString);
        }

        internal static new WSEndpoint ParseIce2Endpoint(
            Transport transport,
            string host,
            ushort port,
            Dictionary<string, string> options,
            Communicator communicator,
            bool oaEndpoint)
        {
            Debug.Assert(transport == Transport.WS || transport == Transport.WSS);

            string? resource = null;

            if (options.TryGetValue("resource", out string? value))
            {
                // The resource value (as supplied in a URI string) must be percent-escaped with '/' separators
                // We keep it as-is, and will marshal it as-is.
                resource = value;
                options.Remove("resource");
            }
            else if (options.TryGetValue("option", out value))
            {
                // We are parsing a ice+universal endpoint
                if (value.Contains(','))
                {
                    throw new FormatException("an ice+ws endpoint accepts at most one marshaled option (resource)");
                }
                // Each option of a universal endpoint needs to be unescaped
                resource = Uri.UnescapeDataString(value);
                options.Remove("option");
            }

            var data = new EndpointData(transport,
                                        host,
                                        port,
                                        resource == null ? Array.Empty<string>() : new string[] { resource });

            return new WSEndpoint(data, options, communicator, oaEndpoint);
        }

        internal override SingleStreamSocket CreateSocket(
            EndPoint addr,
            INetworkProxy? proxy,
            bool preferNonSecure) =>
            new WSSocket(Communicator, base.CreateSocket(addr, proxy, preferNonSecure), Host, Resource);

        internal override SingleStreamSocket CreateSocket(Socket socket, string adapterName, bool preferNonSecure) =>
            new WSSocket(Communicator, base.CreateSocket(socket, adapterName, preferNonSecure));

        protected internal override Connection CreateConnection(
            MultiStreamOverSingleStreamSocket socket,
            object? label,
            ObjectAdapter? adapter) =>
            new WSConnection(this, socket, label, adapter);

        // Constructor used for ice2 parsing.
        private WSEndpoint(
            EndpointData data,
            Dictionary<string, string> options,
            Communicator communicator,
            bool oaEndpoint)
            : base(data, options, communicator, oaEndpoint)
        {
        }

        // Constructor for ice1 parsing
        private WSEndpoint(
            EndpointData data,
            TimeSpan timeout,
            bool compress,
            Dictionary<string, string?> options,
            Communicator communicator,
            bool oaEndpoint,
            string endpointString)
            : base(data, timeout, compress, options, communicator, oaEndpoint, endpointString)
        {
        }

        // Constructor for ice1 unmarshaling
        private WSEndpoint(EndpointData data, TimeSpan timeout, bool compress, Communicator communicator)
            : base(data, timeout, compress, communicator)
        {
        }

        // Constructor for ice2 unmarshaling.
        private WSEndpoint(EndpointData data, Communicator communicator)
            : base(data, communicator)
        {
        }

        // Clone constructor
        private WSEndpoint(WSEndpoint endpoint, string host, ushort port)
            : base(endpoint, host, port)
        {
        }

        private protected override IPEndpoint Clone(string host, ushort port) =>
            new WSEndpoint(this, host, port);
    }
}
