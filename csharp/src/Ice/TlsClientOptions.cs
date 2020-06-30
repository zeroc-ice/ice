//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Net.Security;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;

namespace ZeroC.Ice
{
    /// <summary>This class represents the client side TLS configuration.</summary>
    public class TlsClientOptions
    {
        /// <summary>Gets or sets a value that specifies the mode used to check for X509 certificate revocation. The
        /// default is <see cref="X509RevocationMode.NoCheck"/> that indicates no revocation check is performed on the
        /// certificate.</summary>
        public X509RevocationMode? CertificateRevocationCheckMode
        {
            get => _certificateRevocationCheckMode;
            set
            {
                if (ServerCertificateValidationCallback != null)
                {
                    throw new ArgumentException(
                        @$"Using the {nameof(CertificateRevocationCheckMode)} is incompatible with using the {
                           nameof(ServerCertificateValidationCallback)}");
                }
                _certificateRevocationCheckMode = value;
            }
        }
        /// <summary>A collection of X509 certificates to use by client side connections.</summary>
        public X509Certificate2Collection? ClientCertificates { get; set; }
        /// <summary>A certificate selection callback used to select the client side certificate.</summary>
        public LocalCertificateSelectionCallback? ClientCertificateSelectionCallback { get; set; }

        /// <summary>The list of SSL protocols to enable for outgoing connections, the default is None that allows
        /// the OS to choose the best protocol to use. It can be set to Tls12, Tls13, None or a combination of those,
        /// other values are not accepted.</summary>
        public SslProtocols? EnabledSslProtocols { get; set; }

        /// <summary>Gets or sets the certificates collection that will be used as trusted certificate authorities
        /// to verify the server certificate used for authentication. Setting this is incompatible with setting
        /// <see cref="ServerCertificateValidationCallback"/> and IceSSL.CAs configuration property.</summary>
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

        /// <summary>Gets or sets the callback that will be used to verify the client certificate used for
        /// authentication. Setting this is incompatible with setting
        /// <see cref="ServerCertificateCertificateAuthorities"/> and IceSSL.CAs configuration property.</summary>
        public RemoteCertificateValidationCallback? ServerCertificateValidationCallback
        {
            get => _serverCertificateValidationCallback;
            set
            {
                if (CertificateRevocationCheckMode != null)
                {
                    throw new ArgumentException(
                        @$"Using the {nameof(ServerCertificateValidationCallback)} is incompatible with using the {
                           nameof(CertificateRevocationCheckMode)}");
                }

                if (ServerCertificateCertificateAuthorities != null)
                {
                    throw new ArgumentException(
                        @$"Using the {nameof(ServerCertificateValidationCallback)} is incompatible with using the {
                           nameof(ServerCertificateCertificateAuthorities)}");
                }

                _serverCertificateValidationCallback = value;
            }
        }

        private X509RevocationMode? _certificateRevocationCheckMode;
        private X509Certificate2Collection? _serverCertificateCertificateAuthorities;
        private RemoteCertificateValidationCallback? _serverCertificateValidationCallback;
    }
}
