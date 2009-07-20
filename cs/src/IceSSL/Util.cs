// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    using System;
    using System.Diagnostics;
    using System.Security.Cryptography.X509Certificates;

    /// <summary>
    /// This class provides information about a connection to applications
    /// that require information about a peer, for example, to implement
    /// a CertificateVerifier.
    /// </summary>
    public sealed class ConnectionInfo
    {
        /// <summary>
        /// The certificate chain. This may be null if the peer did not
        /// supply a certificate. The peer's certificate (if any) is the
        /// first one in the chain.
        /// </summary>
        public System.Security.Cryptography.X509Certificates.X509Certificate2[] certs;

        /// <summary>
        /// The name of the negotiated cipher.
        /// </summary>
        public string cipher;

        /// <summary>
        /// The local TCP host and port.
        /// </summary>
        public System.Net.IPEndPoint localAddr;

        /// <summary>
        /// The remote TCP host and port.
        /// </summary>
        public System.Net.IPEndPoint remoteAddr;

        /// <summary>
        /// True if the connection is incoming; false otherwise.
        /// </summary>
        public bool incoming;
        
        /// <summary>
        /// The name of the object adapter that hosts this endpoint, if
        /// any.
        /// </summary>
        public string adapterName;
    }

    /// <summary>
    /// Indicates that a connection is not an SSL connection.
    /// </summary>
    public class ConnectionInvalidException : Ice.LocalException
    {
        #region Slice data members

        /// <summary>
        /// The reason why the connection is considered invalid.
        /// </summary>
        public string reason;

        #endregion

        #region Constructors

        /// <summary>
        /// Creates an instance with an empty reason code field.
        /// </summary>
        public ConnectionInvalidException()
        {
        }

        /// <summary>
        /// Creates an instance with specified reason code field.
        /// </summary>
        public ConnectionInvalidException(System.Exception ex__) : base(ex__)
        {
        }

        #endregion

        /// <summary>
        /// Returns the name of this exception.
        /// </summary>
        /// <returns>Returns "Ice::ConnectionInvalidException".</returns>
        public override string ice_name()
        {
            return "IceSSL::ConnectionInvalidException";
        }

        #region Object members

        public override int GetHashCode()
        {
            int h__ = 0;
            if((object)reason != null)
            {
                h__ = 5 * h__ + reason.GetHashCode();
            }
            return h__;
        }

        public override bool Equals(object other__)
        {
            if(other__ == null)
            {
                return false;
            }
            if(object.ReferenceEquals(this, other__))
            {
                return true;
            }
            if(!(other__ is ConnectionInvalidException))
            {
                return false;
            }
            if(reason == null)
            {
                if(((ConnectionInvalidException)other__).reason != null)
                {
                    return false;
                }
            }
            else
            {
                if(!reason.Equals(((ConnectionInvalidException)other__).reason))
                {
                    return false;
                }
            }
            return true;
        }

        #endregion

        #region Comparison members

        public static bool operator==(ConnectionInvalidException lhs__, ConnectionInvalidException rhs__)
        {
            return Equals(lhs__, rhs__);
        }

        public static bool operator!=(ConnectionInvalidException lhs__, ConnectionInvalidException rhs__)
        {
            return !Equals(lhs__, rhs__);
        }

        #endregion
    }

    public sealed class Util
    {
        public static ConnectionInfo getConnectionInfo(Ice.Connection connection)
        {
            Ice.ConnectionI con = (Ice.ConnectionI)connection;
            Debug.Assert(con != null);

            //
            // Lock the connection directly. This is done because the only
            // thing that prevents the transceiver from being closed during
            // the duration of the invocation is the connection.
            //
            lock(con)
            {
                IceInternal.Transceiver transceiver = con.getTransceiver();
                if(transceiver == null)
                {
                    ConnectionInvalidException ex = new ConnectionInvalidException();
                    ex.reason = "connection closed";
                    throw ex;
                }

                try
                {
                    TransceiverI sslTransceiver = (TransceiverI)transceiver;
                    return sslTransceiver.getConnectionInfo();
                }
                catch(InvalidCastException)
                {
                    ConnectionInvalidException e = new ConnectionInvalidException();
                    e.reason = "not ssl connection";
                    throw e;
                }
            }
        }

        public static X509Certificate2 createCertificate(string certPEM)
        {
            char[] chars = certPEM.ToCharArray();
            byte[] bytes = new byte[chars.Length];
            for(int i = 0; i < chars.Length; ++i)
            {
                bytes[i] = (byte)chars[i];
            }
            return new X509Certificate2(bytes);
        }

        internal static ConnectionInfo
        populateConnectionInfo(System.Net.Security.SslStream stream, System.Net.Sockets.Socket fd,
                               X509Certificate2[] certs, string adapterName, bool incoming)
        {
            ConnectionInfo info = new ConnectionInfo();
            info.certs = certs;
            info.cipher = stream.CipherAlgorithm.ToString();
            try
            {
                info.localAddr = (System.Net.IPEndPoint)fd.LocalEndPoint;
                info.remoteAddr = (System.Net.IPEndPoint)fd.RemoteEndPoint;
            }
            catch(System.Net.Sockets.SocketException ex)
            {
                throw new Ice.SocketException(ex);
            }
            info.incoming = incoming;
            info.adapterName = adapterName;
            return info;
        }
    }
}
