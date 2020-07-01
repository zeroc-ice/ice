//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.IO;
using System.Net.Security;
using System.Security.Authentication;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;

namespace ZeroC.Ice
{
    internal class SslEngine
    {
        // Specifies the mode used to check for X509 certificate revocation when validating client certificates.
        internal X509RevocationMode ClientCertificateRevocationCheckMode { get; }

        // A collection of X509 certificates to use by outgoing connections
        internal X509Certificate2Collection? ClientCertificates;

        // A certificate selection callback used to select the client side certificate.
        internal LocalCertificateSelectionCallback? ClientCertificateSelectionCallback { get; }

        // The list of SSL protocols to enable for outgoing connections.
        internal SslProtocols ClientEnabledSslProtocols { get; }

        // The certificates collection that will be used as trusted certificate authorities to verify the client
        // certificate.
        internal X509Certificate2Collection? ClientCertificateCertificateAuthorities { get; }

        // The callback that will be used to verify the client certificates.
        internal RemoteCertificateValidationCallback? ClientCertificateValidationCallback { get; }

        // A boolean value indicating whenever or not a client certificate is required by the server
        internal bool RequireClientCertificate { get; }

        // Specifies the mode used to check for X509 certificate revocation when validating server certificates.
        internal X509RevocationMode ServerCertificateRevocationCheckMode { get; }

        // An X509 certificate to use by incoming connections
        internal X509Certificate2? ServerCertificate;

        // The list of SSL protocols to enable for incoming connections.
        internal SslProtocols ServerEnabledSslProtocols { get; }

        // The certificates collection that will be used as trusted certificate authorities to verify the server
        // certificate.
        internal X509Certificate2Collection? ServerCertificateCertificateAuthorities { get; }

        // The callback that will be used to verify the server certificates.
        internal RemoteCertificateValidationCallback? ServerCertificateValidationCallback { get; }

        internal int SecurityTraceLevel { get; }
        internal string SecurityTraceCategory => "Security";

        internal SslTrustManager SslTrustManager { get; }
        internal bool UseMachineContext { get; }

        private readonly ILogger _logger;

        internal SslEngine(
            Communicator communicator,
            TlsClientOptions? tlsClientOptions,
            TlsServerOptions? tlsServerOptions)
        {
            _logger = communicator.Logger;
            SecurityTraceLevel = communicator.GetPropertyAsInt("IceSSL.Trace.Security") ?? 0;
            SslTrustManager = new SslTrustManager(communicator);

            UseMachineContext = communicator.GetPropertyAsBool("IceSSL.UseMachineContext") ?? false;

            ClientEnabledSslProtocols = tlsClientOptions?.EnabledSslProtocols ??
                ParseProtocols(communicator.GetPropertyAsList("IceSSL.Protocols"));
            ServerEnabledSslProtocols = tlsServerOptions?.EnabledSslProtocols ??
                ParseProtocols(communicator.GetPropertyAsList("IceSSL.Protocols"));

            ClientCertificateRevocationCheckMode =
                tlsClientOptions?.CertificateRevocationCheckMode ?? X509RevocationMode.NoCheck;
            ServerCertificateRevocationCheckMode =
                tlsServerOptions?.CertificateRevocationCheckMode ?? X509RevocationMode.NoCheck;

            RequireClientCertificate = tlsServerOptions?.RequireClientCertificate ?? true;

            // Check for a default directory. We look in this directory for files mentioned in the configuration.
            string defaultDir = communicator.GetProperty("IceSSL.DefaultDir") ?? "";
            X509Certificate2Collection? certificates = null;
            // If IceSSL.CertFile is defined, load a certificate from a file and add it to the collection.
            // TODO: tracing?

            if (communicator.GetProperty("IceSSL.CertFile") is string certificateFile)
            {
                if (!CheckPath(defaultDir, ref certificateFile))
                {
                    throw new FileNotFoundException(
                        $"certificate file not found: `{certificateFile}'", certificateFile);
                }

                certificates = new X509Certificate2Collection();
                try
                {
                    X509KeyStorageFlags importFlags =
                        UseMachineContext ? X509KeyStorageFlags.MachineKeySet : X509KeyStorageFlags.UserKeySet;
                    certificates.Add(communicator.GetProperty("IceSSL.Password") is string password ?
                        new X509Certificate2(certificateFile, password, importFlags) :
                        new X509Certificate2(certificateFile, "", importFlags));
                }
                catch (CryptographicException ex)
                {
                    throw new InvalidConfigurationException(
                        $"error while attempting to load certificate from `{certificateFile}'", ex);
                }
            }

            ClientCertificates = tlsClientOptions?.ClientCertificates ?? certificates;
            ServerCertificate = tlsServerOptions?.ServerCertificate ?? certificates?[0];

            ClientCertificateSelectionCallback = tlsClientOptions?.ClientCertificateSelectionCallback;

            X509Certificate2Collection? caCertificates = null;

            if (communicator.GetProperty("IceSSL.CAs") is string certAuthFile)
            {
                if (!CheckPath(defaultDir, ref certAuthFile))
                {
                    throw new FileNotFoundException("CA certificate file not found: `{certAuthFile}'",
                        certAuthFile);
                }

                try
                {
                    using FileStream fs = File.OpenRead(certAuthFile);
                    byte[] data = new byte[fs.Length];
                    fs.Read(data, 0, data.Length);

                    string strbuf = "";
                    try
                    {
                        strbuf = System.Text.Encoding.UTF8.GetString(data);
                    }
                    catch (Exception)
                    {
                        // Ignore
                    }

                    caCertificates = new X509Certificate2Collection();
                    if (strbuf.Length == data.Length)
                    {
                        int size, startpos, endpos = 0;
                        bool first = true;
                        while (true)
                        {
                            startpos = strbuf.IndexOf("-----BEGIN CERTIFICATE-----", endpos);
                            if (startpos != -1)
                            {
                                endpos = strbuf.IndexOf("-----END CERTIFICATE-----", startpos);
                                if (endpos == -1)
                                {
                                    throw new InvalidConfigurationException(
                                        $"error while attempting to load certificate from `{certAuthFile}'");
                                }
                                size = endpos - startpos + "-----END CERTIFICATE-----".Length;
                            }
                            else if (first)
                            {
                                startpos = 0;
                                endpos = strbuf.Length;
                                size = strbuf.Length;
                            }
                            else
                            {
                                break;
                            }

                            byte[] cert = new byte[size];
                            Buffer.BlockCopy(data, startpos, cert, 0, size);
                            caCertificates.Import(cert);
                            first = false;
                        }
                    }
                    else
                    {
                        caCertificates.Import(data);
                    }
                }
                catch (Exception ex)
                {
                    throw new InvalidConfigurationException(
                        $"error while attempting to load CA certificate from {certAuthFile}", ex);
                }
            }

            if (tlsClientOptions?.ServerCertificateValidationCallback == null)
            {
                ServerCertificateCertificateAuthorities =
                    tlsClientOptions?.ServerCertificateCertificateAuthorities ?? caCertificates;
            }
            else
            {
                ServerCertificateValidationCallback = tlsClientOptions.ServerCertificateValidationCallback;
            }

            if (tlsServerOptions?.ClientCertificateValidationCallback == null)
            {
                ClientCertificateCertificateAuthorities =
                    tlsServerOptions?.ClientCertificateCertificateAuthorities ?? caCertificates;
            }
            else
            {
                ClientCertificateValidationCallback = tlsServerOptions.ClientCertificateValidationCallback;
            }
        }

        internal void TraceStream(SslStream stream, string description)
        {
            var s = new System.Text.StringBuilder();
            s.Append("SSL connection summary");
            if (description.Length > 0)
            {
                s.Append("\n").Append(description);
            }
            s.Append("\nauthenticated = ").Append(stream.IsAuthenticated ? "yes" : "no");
            s.Append("\nencrypted = ").Append(stream.IsEncrypted ? "yes" : "no");
            s.Append("\nsigned = ").Append(stream.IsSigned ? "yes" : "no");
            s.Append("\nmutually authenticated = ").Append(stream.IsMutuallyAuthenticated ? "yes" : "no");
            s.Append("\nhash algorithm = ").Append(stream.HashAlgorithm).Append("/").Append(stream.HashStrength);
            s.Append("\ncipher algorithm = ").Append(stream.CipherAlgorithm).Append("/").Append(stream.CipherStrength);
            s.Append("\nkey exchange algorithm = ").Append(stream.KeyExchangeAlgorithm).Append("/").Append(
                stream.KeyExchangeStrength);
            s.Append("\nprotocol = ").Append(stream.SslProtocol);
            _logger.Trace(SecurityTraceCategory, s.ToString());
        }

        private static bool IsAbsolutePath(string path)
        {
            // Skip whitespace
            path = path.Trim();

            if (AssemblyUtil.IsWindows)
            {
                // We need at least 3 non-whitespace characters to have an absolute path
                if (path.Length < 3)
                {
                    return false;
                }

                // Check for X:\ path ('\' may have been converted to '/')
                if ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z'))
                {
                    return path[1] == ':' && (path[2] == '\\' || path[2] == '/');
                }
            }

            // Check for UNC path
            return (path[0] == '\\' && path[1] == '\\') || path[0] == '/';
        }

        private static SslProtocols ParseProtocols(string[]? arr)
        {
            SslProtocols result = SslProtocols.None;

            if (arr != null && arr.Length > 0)
            {
                result = 0;
                for (int i = 0; i < arr.Length; ++i)
                {
                    string protocol;
                    string s = arr[i].ToUpperInvariant();
                    switch (s)
                    {
                        case "TLS1_2":
                        case "TLSV1_2":
                        {
                            protocol = "Tls12";
                            break;
                        }
                        case "TLS1_3":
                        case "TLSV1_3":
                        {
                            protocol = "Tls13";
                            break;
                        }
                        default:
                        {
                            throw new FormatException($"unrecognized SSL protocol `{s}'");
                        }
                    }

                    try
                    {
                        var value = (SslProtocols)Enum.Parse(typeof(SslProtocols), protocol);
                        result |= value;
                    }
                    catch (Exception ex)
                    {
                        throw new FormatException($"unrecognized SSL protocol `{s}'", ex);
                    }
                }
            }
            return result;
        }

        private bool CheckPath(string defaultDir, ref string path)
        {
            if (File.Exists(path))
            {
                return true;
            }

            if (defaultDir.Length > 0 && !IsAbsolutePath(path))
            {
                string s = Path.Combine(defaultDir, path);
                if (File.Exists(s))
                {
                    path = s;
                    return true;
                }
            }

            return false;
        }
    }
}
