//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Net.Security;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;

namespace ZeroC.Ice
{
    /// <summary>This class represents the server side TLS configuration.</summary>
    public class TlsServerOptions
    {
        /// <summary>Gets or sets a value that specifies the mode used to check for X509 certificate revocation. The
        /// default is <see cref="X509RevocationMode.NoCheck"/> that indicates no revocation check is performed on the
        /// certificate.</summary>
        public X509RevocationMode? CertificateRevocationCheckMode
        {
            get => _certificateRevocationCheckMode;
            set
            {
                if (ClientCertificateValidationCallback != null)
                {
                    throw new ArgumentException(
                        @$"Using the {nameof(CertificateRevocationCheckMode)} is incompatible with using the {
                           nameof(ClientCertificateValidationCallback)}");
                }
                _certificateRevocationCheckMode = value;
            }
        }

        /// <summary>Gets or sets the certificates collection that will be used as trusted certificate authorities
        /// to verify the client certificate used for authentication. Setting this is incompatible with setting
        /// <see cref="ClientCertificateValidationCallback"/> and IceSSL.CAs configuration property.</summary>
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
        /// <summary>Gets or sets the callback that will be used to verify the client certificate used for
        /// authentication. Setting this is incompatible with setting
        /// <see cref="ClientCertificateCertificateAuthorities"/> and IceSSL.CAs configuration property.</summary>
        public RemoteCertificateValidationCallback? ClientCertificateValidationCallback
        {
            get => _clientCertificateValidationCallback;
            set
            {
                if (CertificateRevocationCheckMode != null)
                {
                    throw new ArgumentException(
                        @$"Using the {nameof(ClientCertificateValidationCallback)} is incompatible with using the {
                           nameof(CertificateRevocationCheckMode)}");
                }

                if (ClientCertificateCertificateAuthorities != null)
                {
                    throw new ArgumentException(
                        @$"Using the {nameof(ClientCertificateValidationCallback)} is incompatible with using the {
                           nameof(ClientCertificateCertificateAuthorities)}");
                }

                _clientCertificateValidationCallback = value;
            }
        }
        /// <summary>The list of SSL protocols to enable for server side endpoints, the default is None that allows
        /// the OS to choose the best protocol to use. It can be set to Tls12, Tls13, None or a combination of those,
        /// other values are not accepted.</summary>
        public SslProtocols? EnabledSslProtocols { get; set; }

        /// <summary>Gets or sets a boolean value that specifies whether the client is asked for a certificate for
        /// authentication. Note that this is only a request -- if no certificate is provided, the server still accepts
        /// the connection request.</summary>
        public bool RequireClientCertificate { get; set; }
        /// <summary>A collection of X509 certificates to use by server side connections.</summary>
        public X509Certificate2? ServerCertificate { get; set; }
        /// <summary>A certificate selection callback used to select the server side certificate.</summary>
        public LocalCertificateSelectionCallback? ServerCertificateSelectionCallback { get; set; }

        private X509RevocationMode? _certificateRevocationCheckMode;
        private RemoteCertificateValidationCallback? _clientCertificateValidationCallback;
        private X509Certificate2Collection? _clientCertificateCertificateAuthorities;
    }
}
