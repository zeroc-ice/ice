// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Net.Security;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;

namespace ZeroC.Ice
{
    /// <summary>This class represents the server side TLS configuration.</summary>
    public class TlsServerOptions
    {
        /// <summary>Gets or sets the certificates of trusted certificate authorities. These authorities are used to
        /// verify client authentication. Setting this is incompatible with setting
        /// <see cref="ClientCertificateValidationCallback"/>.</summary>
        public X509Certificate2Collection? ClientCertificateCertificateAuthorities
        {
            get => _clientCertificateCertificateAuthorities;
            set
            {
                if (ClientCertificateValidationCallback != null)
                {
                    throw new ArgumentException(
                        @$"Using the {nameof(ClientCertificateCertificateAuthorities)} is incompatible with using the {
                           nameof(ClientCertificateValidationCallback)}");
                }
                _clientCertificateCertificateAuthorities = value;
            }
        }

        /// <summary>Gets or sets the callback that will be used to verify the client certificate. Setting this is
        /// incompatible with setting <see cref="ClientCertificateCertificateAuthorities"/>.</summary>
        public RemoteCertificateValidationCallback? ClientCertificateValidationCallback
        {
            get => _clientCertificateValidationCallback;
            set
            {
                if (ClientCertificateCertificateAuthorities != null)
                {
                    throw new ArgumentException(
                        @$"Using the {nameof(ClientCertificateValidationCallback)} is incompatible with using the {
                           nameof(ClientCertificateCertificateAuthorities)}");
                }

                _clientCertificateValidationCallback = value;
            }
        }

        /// <summary>Gets or sets the list of SSL protocols allowed for server connections. The default is None which
        /// specifies to rely on the operating system for selecting the best protocol to use. It can be set to Tls12,
        /// Tls13, None or a combination of these, other values are not accepted.</summary>
        public SslProtocols? EnabledSslProtocols { get; set; }

        /// <summary>Gets or sets a boolean value to specify if the client certificate is requested. If no custom
        /// client certificate validation callback is set, the built-in validation will fail if the client doesn't
        /// provide a certificate and one is required.</summary>
        public bool RequireClientCertificate { get; set; }

        /// <summary>When true and IceSSL.CertFile property is defined, the certificates are imported with the
        /// <see cref="X509KeyStorageFlags.MachineKeySet"/> flag and the certificate chain used to validate the
        /// client certificate is build using the machine context, otherwise the certificates are imported with the
        /// <see cref="X509KeyStorageFlags.UserKeySet"/> and the certificate chain uses the user context. On Linux
        /// certificates are always imported with <see cref="X509KeyStorageFlags.EphemeralKeySet"/> flag independently
        /// of the value of this property.</summary>
        public bool UseMachineContex { get; set; }

        /// <summary>Gets or sets the certificate user for incoming connections.</summary>
        public X509Certificate2? ServerCertificate { get; set; }
        private RemoteCertificateValidationCallback? _clientCertificateValidationCallback;
        private X509Certificate2Collection? _clientCertificateCertificateAuthorities;
    }
}
