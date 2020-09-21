// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Net.Security;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;

namespace ZeroC.Ice
{
    /// <summary>This class represents the client side TLS configuration.</summary>
    public class TlsClientOptions
    {
        /// <summary>Gets or sets the certificates used to authenticate client connections. If the
        /// <see cref="ClientCertificateSelectionCallback"/> is defined, the certificates are provided to the callback.
        /// Otherwise, the built-in certificate selection callback will select a certificate that matches one of the
        /// server's issuers or the first certificate if no matching issuers is found.</summary>
        public X509Certificate2Collection? ClientCertificates { get; set; }

        /// <summary>Gets or set the certificate selection callback used to select the client side certificate.
        /// </summary>
        public LocalCertificateSelectionCallback? ClientCertificateSelectionCallback { get; set; }

        /// <summary>Gets or sets the list of SSL protocols allowed for client connections. The default is None which
        /// specifies to rely on the operating system for selecting the best protocol to use. It can be set to Tls12,
        /// Tls13, None or a combination of these, other values are not accepted.</summary>
        public SslProtocols? EnabledSslProtocols { get; set; }

        /// <summary>Gets or sets the certificates collection that will be used as trusted certificate authorities
        /// to verify the server certificate. Setting this is incompatible with setting
        /// <see cref="ServerCertificateValidationCallback"/>.</summary>
        public X509Certificate2Collection? ServerCertificateCertificateAuthorities
        {
            get => _serverCertificateCertificateAuthorities;
            set
            {
                if (ServerCertificateValidationCallback != null)
                {
                    throw new ArgumentException(
                        @$"Using the {nameof(ServerCertificateCertificateAuthorities)} is incompatible with using the {
                           nameof(ServerCertificateValidationCallback)}");
                }
                _serverCertificateCertificateAuthorities = value;
            }
        }

        /// <summary>Gets or sets the callback that will be used to verify the server certificate. Setting this is
        /// incompatible with setting <see cref="ServerCertificateCertificateAuthorities"/>.</summary>
        public RemoteCertificateValidationCallback? ServerCertificateValidationCallback
        {
            get => _serverCertificateValidationCallback;
            set
            {
                if (ServerCertificateCertificateAuthorities != null)
                {
                    throw new ArgumentException(
                        @$"Using the {nameof(ServerCertificateValidationCallback)} is incompatible with using the {
                           nameof(ServerCertificateCertificateAuthorities)}");
                }

                _serverCertificateValidationCallback = value;
            }
        }

        /// <summary>When true and IceSSL.CertFile property is defined, the certificates are imported with the
        /// <see cref="X509KeyStorageFlags.MachineKeySet"/> flag and the certificate chain used to validate the
        /// server certificate is build using the machine context, otherwise the certificates are imported with the
        /// <see cref="X509KeyStorageFlags.UserKeySet"/> and the certificate chain uses the user context. On Linux
        /// certificates are always imported with <see cref="X509KeyStorageFlags.EphemeralKeySet"/> flag independently
        /// of the value of this property.</summary>
        public bool UseMachineContex { get; set; }

        private X509Certificate2Collection? _serverCertificateCertificateAuthorities;
        private RemoteCertificateValidationCallback? _serverCertificateValidationCallback;
    }
}
