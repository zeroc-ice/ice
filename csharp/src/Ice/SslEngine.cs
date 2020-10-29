// Copyright (c) ZeroC, Inc. All rights reserved.

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
        // TLS Client side configuration
        internal TlsClientOptions TlsClientOptions { get; }

        // TLS Server side configuration
        internal TlsServerOptions TlsServerOptions { get; }

        internal int SecurityTraceLevel { get; }
        internal const string SecurityTraceCategory = "Security";

        private readonly ILogger _logger;

        internal SslEngine(
            Communicator communicator,
            TlsClientOptions? tlsClientOptions,
            TlsServerOptions? tlsServerOptions)
        {
            _logger = communicator.Logger;
            SecurityTraceLevel = communicator.GetPropertyAsInt("IceSSL.Trace.Security") ?? 0;

            TlsClientOptions = new TlsClientOptions();
            TlsServerOptions = new TlsServerOptions();

            TlsClientOptions.EnabledSslProtocols = tlsClientOptions?.EnabledSslProtocols ??
                ParseProtocols(communicator.GetPropertyAsList("IceSSL.Protocols"));
            TlsServerOptions.EnabledSslProtocols = tlsServerOptions?.EnabledSslProtocols ??
                ParseProtocols(communicator.GetPropertyAsList("IceSSL.Protocols"));

            TlsServerOptions.RequireClientCertificate = tlsServerOptions?.RequireClientCertificate ?? true;

            // Check for a default directory. We look in this directory for files mentioned in the configuration.
            string defaultDir = communicator.GetProperty("IceSSL.DefaultDir") ?? "";
            X509Certificate2Collection? certificates = null;
            // If IceSSL.CertFile is defined, load a certificate from a file and add it to the collection.
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
                    X509KeyStorageFlags importFlags;
                    if (OperatingSystem.IsLinux())
                    {
                        importFlags = X509KeyStorageFlags.EphemeralKeySet;
                    }
                    else
                    {
                        importFlags = tlsClientOptions?.UseMachineContex ??
                                      tlsServerOptions?.UseMachineContex ?? false ?
                            X509KeyStorageFlags.MachineKeySet : X509KeyStorageFlags.UserKeySet;
                    }

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

            TlsClientOptions.ClientCertificates = tlsClientOptions?.ClientCertificates ?? certificates;
            TlsServerOptions.ServerCertificate = tlsServerOptions?.ServerCertificate ?? certificates?[0];

            TlsClientOptions.ClientCertificateSelectionCallback = tlsClientOptions?.ClientCertificateSelectionCallback;

            X509Certificate2Collection? caCertificates = null;

            if (communicator.GetProperty("IceSSL.CAs") is string certAuthFile)
            {
                if (!CheckPath(defaultDir, ref certAuthFile))
                {
                    throw new FileNotFoundException($"CA certificate file not found: `{certAuthFile}'", certAuthFile);
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

                    string beginCertificateMark = "-----BEGIN CERTIFICATE-----";
                    string endCertificateMark = "-----END CERTIFICATE-----";

                    caCertificates = new X509Certificate2Collection();
                    if (strbuf.Length == data.Length)
                    {
                        int size, startpos, endpos = 0;
                        bool first = true;
                        while (true)
                        {
                            startpos = strbuf.IndexOf(beginCertificateMark, endpos, StringComparison.InvariantCulture);
                            if (startpos != -1)
                            {
                                endpos = strbuf.IndexOf(endCertificateMark, startpos, StringComparison.InvariantCulture);
                                if (endpos == -1)
                                {
                                    throw new FormatException(
                                        $"end certificate mark `{endCertificateMark}' not found");
                                }
                                size = endpos - startpos + endCertificateMark.Length;
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
                        $"error while attempting to load CA certificate from `{certAuthFile}'", ex);
                }
            }

            if (tlsClientOptions?.ServerCertificateValidationCallback == null)
            {
                TlsClientOptions.ServerCertificateCertificateAuthorities =
                    tlsClientOptions?.ServerCertificateCertificateAuthorities ?? caCertificates;
            }
            else
            {
                TlsClientOptions.ServerCertificateValidationCallback =
                    tlsClientOptions.ServerCertificateValidationCallback;
            }

            if (tlsServerOptions?.ClientCertificateValidationCallback == null)
            {
                TlsServerOptions.ClientCertificateCertificateAuthorities =
                    tlsServerOptions?.ClientCertificateCertificateAuthorities ?? caCertificates;
            }
            else
            {
                TlsServerOptions.ClientCertificateValidationCallback =
                    tlsServerOptions.ClientCertificateValidationCallback;
            }
        }

        internal void TraceStream(SslStream stream, string description)
        {
            var s = new System.Text.StringBuilder();
            s.Append("SSL connection summary");
            if (description.Length > 0)
            {
                s.Append('\n').Append(description);
            }
            s.Append("\nauthenticated = ").Append(stream.IsAuthenticated ? "yes" : "no");
            s.Append("\nencrypted = ").Append(stream.IsEncrypted ? "yes" : "no");
            s.Append("\nsigned = ").Append(stream.IsSigned ? "yes" : "no");
            s.Append("\nmutually authenticated = ").Append(stream.IsMutuallyAuthenticated ? "yes" : "no");
            s.Append("\nhash algorithm = ").Append(stream.HashAlgorithm).Append('/').Append(stream.HashStrength);
            s.Append("\ncipher algorithm = ").Append(stream.CipherAlgorithm).Append('/').Append(stream.CipherStrength);
            s.Append("\nkey exchange algorithm = ").Append(stream.KeyExchangeAlgorithm).Append('/').Append(
                stream.KeyExchangeStrength);
            s.Append("\nprotocol = ").Append(stream.SslProtocol);
            _logger.Trace(SecurityTraceCategory, s.ToString());
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

        private static bool CheckPath(string defaultDir, ref string path)
        {
            if (File.Exists(path))
            {
                return true;
            }

            string s = Path.Combine(defaultDir, path);
            if (s != path && File.Exists(s))
            {
                path = s;
                return true;
            }

            return false;
        }
    }
}
