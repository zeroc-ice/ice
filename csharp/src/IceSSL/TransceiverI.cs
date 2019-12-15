//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.IO;
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;

namespace IceSSL
{
    internal sealed class TransceiverI : IceInternal.Transceiver
    {
        public Socket? fd()
        {
            return _delegate.fd();
        }

        public int initialize(IceInternal.Buffer readBuffer, IceInternal.Buffer writeBuffer, ref bool hasMoreData)
        {
            if (!_isConnected)
            {
                int status = _delegate.initialize(readBuffer, writeBuffer, ref hasMoreData);
                if (status != IceInternal.SocketOperation.None)
                {
                    return status;
                }
                _isConnected = true;
            }

            Socket? fd = _delegate.fd();
            Debug.Assert(fd != null);

            IceInternal.Network.setBlock(fd, true); // SSL requires a blocking socket

            //
            // For timeouts to work properly, we need to receive/send
            // the data in several chunks. Otherwise, we would only be
            // notified when all the data is received/written. The
            // connection timeout could easily be triggered when
            // receiging/sending large messages.
            //
            _maxSendPacketSize = Math.Max(512, IceInternal.Network.getSendBufferSize(fd));
            _maxRecvPacketSize = Math.Max(512, IceInternal.Network.getRecvBufferSize(fd));

            if (_sslStream == null)
            {
                try
                {
                    _sslStream = new SslStream(new NetworkStream(fd, false),
                                               false,
                                               new RemoteCertificateValidationCallback(validationCallback),
                                               new LocalCertificateSelectionCallback(selectCertificate));
                }
                catch (IOException ex)
                {
                    if (IceInternal.Network.connectionLost(ex))
                    {
                        throw new Ice.ConnectionLostException(ex);
                    }
                    else
                    {
                        throw new Ice.SocketException(ex);
                    }
                }
                return IceInternal.SocketOperation.Connect;
            }

            Debug.Assert(_sslStream.IsAuthenticated);
            _authenticated = true;

            _cipher = _sslStream.CipherAlgorithm.ToString();
            _instance.verifyPeer(_host, (ConnectionInfo)getInfo(), ToString());

            if (_instance.securityTraceLevel() >= 1)
            {
                _instance.traceStream(_sslStream, ToString());
            }
            return IceInternal.SocketOperation.None;
        }

        public int closing(bool initiator, Ice.LocalException? ex)
        {
            return _delegate.closing(initiator, ex);
        }

        public void close()
        {
            if (_sslStream != null)
            {
                _sslStream.Close(); // Closing the stream also closes the socket.
                _sslStream = null;
            }

            _delegate.close();
        }

        public IceInternal.EndpointI bind()
        {
            Debug.Assert(false);
            return null;
        }

        public void destroy()
        {
            _delegate.destroy();
        }

        public int write(IceInternal.Buffer buf)
        {
            //
            // Force caller to use async write.
            //
            return buf.b.hasRemaining() ? IceInternal.SocketOperation.Write : IceInternal.SocketOperation.None;
        }

        public int read(IceInternal.Buffer buf, ref bool hasMoreData)
        {
            //
            // Force caller to use async read.
            //
            return buf.b.hasRemaining() ? IceInternal.SocketOperation.Read : IceInternal.SocketOperation.None;
        }

        public bool startRead(IceInternal.Buffer buf, IceInternal.AsyncCallback callback, object state)
        {
            if (!_isConnected)
            {
                return _delegate.startRead(buf, callback, state);
            }

            Debug.Assert(_sslStream != null && _sslStream.IsAuthenticated);

            int packetSz = getRecvPacketSize(buf.b.remaining());
            try
            {
                _readCallback = callback;
                _readResult = _sslStream.BeginRead(buf.b.rawBytes(), buf.b.position(), packetSz, readCompleted, state);
                return _readResult.CompletedSynchronously;
            }
            catch (IOException ex)
            {
                if (IceInternal.Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                if (IceInternal.Network.timeout(ex))
                {
                    throw new Ice.TimeoutException();
                }
                throw new Ice.SocketException(ex);
            }
            catch (ObjectDisposedException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
            catch (Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }
        }

        public void finishRead(IceInternal.Buffer buf)
        {
            if (!_isConnected)
            {
                _delegate.finishRead(buf);
                return;
            }
            else if (_sslStream == null) // Transceiver was closed
            {
                _readResult = null;
                return;
            }

            Debug.Assert(_readResult != null);
            try
            {
                int ret = _sslStream.EndRead(_readResult);
                _readResult = null;

                if (ret == 0)
                {
                    throw new Ice.ConnectionLostException();
                }
                Debug.Assert(ret > 0);
                buf.b.position(buf.b.position() + ret);
            }
            catch (Ice.LocalException)
            {
                throw;
            }
            catch (IOException ex)
            {
                if (IceInternal.Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                if (IceInternal.Network.timeout(ex))
                {
                    throw new Ice.TimeoutException();
                }
                throw new Ice.SocketException(ex);
            }
            catch (ObjectDisposedException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
            catch (Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }
        }

        public bool startWrite(IceInternal.Buffer buf, IceInternal.AsyncCallback cb, object state, out bool completed)
        {
            if (!_isConnected)
            {
                return _delegate.startWrite(buf, cb, state, out completed);
            }

            Debug.Assert(_sslStream != null);
            if (!_authenticated)
            {
                completed = false;
                return startAuthenticate(cb, state);
            }

            //
            // We limit the packet size for beingWrite to ensure connection timeouts are based
            // on a fixed packet size.
            //
            int packetSize = getSendPacketSize(buf.b.remaining());
            try
            {
                _writeCallback = cb;
                _writeResult = _sslStream.BeginWrite(buf.b.rawBytes(), buf.b.position(), packetSize, writeCompleted,
                                                     state);
                completed = packetSize == buf.b.remaining();
                return _writeResult.CompletedSynchronously;
            }
            catch (IOException ex)
            {
                if (IceInternal.Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                if (IceInternal.Network.timeout(ex))
                {
                    throw new Ice.TimeoutException();
                }
                throw new Ice.SocketException(ex);
            }
            catch (ObjectDisposedException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
            catch (Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }
        }

        public void finishWrite(IceInternal.Buffer buf)
        {
            if (!_isConnected)
            {
                _delegate.finishWrite(buf);
                return;
            }
            else if (_sslStream == null) // Transceiver was closed
            {
                if (getSendPacketSize(buf.b.remaining()) == buf.b.remaining()) // Sent last packet
                {
                    buf.b.position(buf.b.limit()); // Assume all the data was sent for at-most-once semantics.
                }
                _writeResult = null;
                return;
            }
            else if (!_authenticated)
            {
                finishAuthenticate();
                return;
            }

            Debug.Assert(_writeResult != null);
            int sent = getSendPacketSize(buf.b.remaining());
            try
            {
                _sslStream.EndWrite(_writeResult);
                _writeResult = null;
                buf.b.position(buf.b.position() + sent);
            }
            catch (IOException ex)
            {
                if (IceInternal.Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                if (IceInternal.Network.timeout(ex))
                {
                    throw new Ice.TimeoutException();
                }
                throw new Ice.SocketException(ex);
            }
            catch (ObjectDisposedException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
            catch (Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }
        }

        public string protocol()
        {
            return _delegate.protocol();
        }

        public Ice.ConnectionInfo getInfo()
        {
            ConnectionInfo info = new ConnectionInfo();
            info.underlying = _delegate.getInfo();
            info.incoming = _incoming;
            info.adapterName = _adapterName;
            info.cipher = _cipher;
            info.certs = _certs;
            info.verified = _verified;
            return info;
        }

        public void checkSendSize(IceInternal.Buffer buf)
        {
            _delegate.checkSendSize(buf);
        }

        public void setBufferSize(int rcvSize, int sndSize)
        {
            _delegate.setBufferSize(rcvSize, sndSize);
        }

        public override string ToString()
        {
            return _delegate.ToString();
        }

        public string toDetailedString()
        {
            return _delegate.toDetailedString();
        }

        //
        // Only for use by ConnectorI, AcceptorI.
        //
        internal TransceiverI(Instance instance, IceInternal.Transceiver del, string hostOrAdapterName, bool incoming)
        {
            _instance = instance;
            _delegate = del;
            _incoming = incoming;
            if (_incoming)
            {
                _adapterName = hostOrAdapterName;
            }
            else
            {
                _host = hostOrAdapterName;
            }

            _sslStream = null;

            _verifyPeer = _instance.properties().getPropertyAsIntWithDefault("IceSSL.VerifyPeer", 2);
        }

        private bool startAuthenticate(IceInternal.AsyncCallback callback, object state)
        {
            Debug.Assert(_sslStream != null);
            try
            {
                _writeCallback = callback;
                if (!_incoming)
                {
                    //
                    // Client authentication.
                    //
                    _writeResult = _sslStream.BeginAuthenticateAsClient(_host,
                                                                        _instance.certs(),
                                                                        _instance.protocols(),
                                                                        _instance.checkCRL() > 0,
                                                                        writeCompleted,
                                                                        state);
                }
                else
                {
                    //
                    // Server authentication.
                    //
                    // Get the certificate collection and select the first one.
                    //
                    X509Certificate2Collection? certs = _instance.certs();
                    X509Certificate2? cert = null;
                    if (certs != null && certs.Count > 0)
                    {
                        cert = certs[0];
                    }

                    _writeResult = _sslStream.BeginAuthenticateAsServer(cert,
                                                                        _verifyPeer > 0,
                                                                        _instance.protocols(),
                                                                        _instance.checkCRL() > 0,
                                                                        writeCompleted,
                                                                        state);
                }
            }
            catch (IOException ex)
            {
                if (IceInternal.Network.connectionLost(ex))
                {
                    //
                    // This situation occurs when connectToSelf is called; the "remote" end
                    // closes the socket immediately.
                    //
                    throw new Ice.ConnectionLostException();
                }
                throw new Ice.SocketException(ex);
            }
            catch (AuthenticationException ex)
            {
                Ice.SecurityException e = new Ice.SecurityException(ex);
                e.reason = ex.Message;
                throw e;
            }
            catch (Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }

            Debug.Assert(_writeResult != null);
            return _writeResult.CompletedSynchronously;
        }

        private void finishAuthenticate()
        {
            Debug.Assert(_writeResult != null);
            Debug.Assert(_sslStream != null);

            try
            {
                if (!_incoming)
                {
                    _sslStream.EndAuthenticateAsClient(_writeResult);
                }
                else
                {
                    _sslStream.EndAuthenticateAsServer(_writeResult);
                }
            }
            catch (IOException ex)
            {
                if (IceInternal.Network.connectionLost(ex))
                {
                    //
                    // This situation occurs when connectToSelf is called; the "remote" end
                    // closes the socket immediately.
                    //
                    throw new Ice.ConnectionLostException();
                }
                throw new Ice.SocketException(ex);
            }
            catch (AuthenticationException ex)
            {
                Ice.SecurityException e = new Ice.SecurityException(ex);
                e.reason = ex.Message;
                throw e;
            }
            catch (Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }
        }

        private X509Certificate? selectCertificate(object sender,
            string targetHost,
            X509CertificateCollection? certs,
            X509Certificate remoteCertificate,
            string[]? acceptableIssuers)
        {
            if (certs == null || certs.Count == 0)
            {
                return null;
            }
            else if (certs.Count == 1)
            {
                return certs[0];
            }

            //
            // Use the first certificate that match the acceptable issuers.
            //
            if (acceptableIssuers != null && acceptableIssuers.Length > 0)
            {
                foreach (X509Certificate certificate in certs)
                {
                    if (Array.IndexOf(acceptableIssuers, certificate.Issuer) != -1)
                    {
                        return certificate;
                    }
                }
            }
            return certs[0];
        }

        private bool validationCallback(object sender, X509Certificate certificate, X509Chain chainEngine,
                                        SslPolicyErrors policyErrors)
        {
            X509Chain chain = new X509Chain(_instance.engine().useMachineContext());
            try
            {
                if (_instance.checkCRL() == 0)
                {
                    chain.ChainPolicy.RevocationMode = X509RevocationMode.NoCheck;
                }

                X509Certificate2Collection? caCerts = _instance.engine().caCerts();
                if (caCerts != null)
                {
                    //
                    // We need to set this flag to be able to use a certificate authority from the extra store.
                    //
                    chain.ChainPolicy.VerificationFlags = X509VerificationFlags.AllowUnknownCertificateAuthority;
                    foreach (X509Certificate2 cert in caCerts)
                    {
                        chain.ChainPolicy.ExtraStore.Add(cert);
                    }
                }

                string message = "";
                int errors = (int)policyErrors;
                if (certificate != null)
                {
                    chain.Build(new X509Certificate2(certificate));
                    if (chain.ChainStatus != null && chain.ChainStatus.Length > 0)
                    {
                        errors |= (int)SslPolicyErrors.RemoteCertificateChainErrors;
                    }
                    else if (_instance.engine().caCerts() != null)
                    {
                        X509ChainElement e = chain.ChainElements[chain.ChainElements.Count - 1];
                        if (!chain.ChainPolicy.ExtraStore.Contains(e.Certificate))
                        {
                            if (_verifyPeer > 0)
                            {
                                message += "\npuntrusted root certificate";
                            }
                            else
                            {
                                message += "\nuntrusted root certificate (ignored)";
                                _verified = false;
                            }
                            errors |= (int)SslPolicyErrors.RemoteCertificateChainErrors;
                        }
                        else
                        {
                            _verified = true;
                            return true;
                        }
                    }
                    else
                    {
                        _verified = true;
                        return true;
                    }
                }

                if ((errors & (int)SslPolicyErrors.RemoteCertificateNotAvailable) > 0)
                {
                    //
                    // The RemoteCertificateNotAvailable case does not appear to be possible
                    // for an outgoing connection. Since .NET requires an authenticated
                    // connection, the remote peer closes the socket if it does not have a
                    // certificate to provide.
                    //

                    if (_incoming)
                    {
                        if (_verifyPeer > 1)
                        {
                            if (_instance.securityTraceLevel() >= 1)
                            {
                                _instance.logger().trace(_instance.securityTraceCategory(),
                                                         "SSL certificate validation failed - client certificate not provided");
                            }
                            return false;
                        }
                        errors ^= (int)SslPolicyErrors.RemoteCertificateNotAvailable;
                        message += "\nremote certificate not provided (ignored)";
                    }
                }

                bool certificateNameMismatch = (errors & (int)SslPolicyErrors.RemoteCertificateNameMismatch) > 0;
                if (certificateNameMismatch)
                {
                    if (_instance.engine().getCheckCertName() && !string.IsNullOrEmpty(_host))
                    {
                        if (_instance.securityTraceLevel() >= 1)
                        {
                            string msg = "SSL certificate validation failed - Hostname mismatch";
                            if (_verifyPeer == 0)
                            {
                                msg += " (ignored)";
                            }
                            _instance.logger().trace(_instance.securityTraceCategory(), msg);
                        }

                        if (_verifyPeer > 0)
                        {
                            return false;
                        }
                        else
                        {
                            errors ^= (int)SslPolicyErrors.RemoteCertificateNameMismatch;
                        }
                    }
                    else
                    {
                        errors ^= (int)SslPolicyErrors.RemoteCertificateNameMismatch;
                        certificateNameMismatch = false;
                    }
                }

                if ((errors & (int)SslPolicyErrors.RemoteCertificateChainErrors) > 0 &&
                   chain.ChainStatus != null && chain.ChainStatus.Length > 0)
                {
                    int errorCount = 0;
                    foreach (X509ChainStatus status in chain.ChainStatus)
                    {
                        if (status.Status == X509ChainStatusFlags.UntrustedRoot && _instance.engine().caCerts() != null)
                        {
                            //
                            // Untrusted root is OK when using our custom chain engine if
                            // the CA certificate is present in the chain policy extra store.
                            //
                            X509ChainElement e = chain.ChainElements[chain.ChainElements.Count - 1];
                            if (!chain.ChainPolicy.ExtraStore.Contains(e.Certificate))
                            {
                                if (_verifyPeer > 0)
                                {
                                    message += "\npuntrusted root certificate";
                                    ++errorCount;
                                }
                                else
                                {
                                    message += "\nuntrusted root certificate (ignored)";
                                }
                            }
                            else
                            {
                                _verified = !certificateNameMismatch;
                            }
                        }
                        else if (status.Status == X509ChainStatusFlags.Revoked)
                        {
                            if (_instance.checkCRL() > 0)
                            {
                                message += "\ncertificate revoked";
                                ++errorCount;
                            }
                            else
                            {
                                message += "\ncertificate revoked (ignored)";
                            }
                        }
                        else if (status.Status == X509ChainStatusFlags.RevocationStatusUnknown)
                        {
                            //
                            // If a certificate's revocation status cannot be determined, the strictest
                            // policy is to reject the connection.
                            //
                            if (_instance.checkCRL() > 1)
                            {
                                message += "\ncertificate revocation status unknown";
                                ++errorCount;
                            }
                            else
                            {
                                message += "\ncertificate revocation status unknown (ignored)";
                            }
                        }
                        else if (status.Status == X509ChainStatusFlags.PartialChain)
                        {
                            if (_verifyPeer > 0)
                            {
                                message += "\npartial certificate chain";
                                ++errorCount;
                            }
                            else
                            {
                                message += "\npartial certificate chain (ignored)";
                            }
                        }
                        else if (status.Status != X509ChainStatusFlags.NoError)
                        {
                            message = message + "\ncertificate chain error: " + status.Status.ToString();
                            ++errorCount;
                        }
                    }

                    if (errorCount == 0)
                    {
                        errors ^= (int)SslPolicyErrors.RemoteCertificateChainErrors;
                    }
                }

                if (errors > 0)
                {
                    if (_instance.securityTraceLevel() >= 1)
                    {
                        if (message.Length > 0)
                        {
                            _instance.logger().trace(_instance.securityTraceCategory(),
                                                     "SSL certificate validation failed:" + message);
                        }
                        else
                        {
                            _instance.logger().trace(_instance.securityTraceCategory(),
                                                     "SSL certificate validation failed");
                        }
                    }
                    return false;
                }
                else if (message.Length > 0 && _instance.securityTraceLevel() >= 1)
                {
                    _instance.logger().trace(_instance.securityTraceCategory(),
                                             "SSL certificate validation status:" + message);
                }
                return true;
            }
            finally
            {
                if (chain.ChainElements != null && chain.ChainElements.Count > 0)
                {
                    _certs = new X509Certificate2[chain.ChainElements.Count];
                    for (int i = 0; i < chain.ChainElements.Count; ++i)
                    {
                        _certs[i] = chain.ChainElements[i].Certificate;
                    }
                }

                try
                {
                    chain.Dispose();
                }
                catch (Exception)
                {
                }
            }
        }

        internal void readCompleted(IAsyncResult result)
        {
            if (!result.CompletedSynchronously)
            {
                Debug.Assert(_readCallback != null);
                _readCallback(result.AsyncState);
            }
        }

        internal void writeCompleted(IAsyncResult result)
        {
            if (!result.CompletedSynchronously)
            {
                Debug.Assert(_writeCallback != null);
                _writeCallback(result.AsyncState);
            }
        }

        private int getSendPacketSize(int length)
        {
            return _maxSendPacketSize > 0 ? Math.Min(length, _maxSendPacketSize) : length;
        }

        public int getRecvPacketSize(int length)
        {
            return _maxRecvPacketSize > 0 ? Math.Min(length, _maxRecvPacketSize) : length;
        }

        private readonly Instance _instance;
        private readonly IceInternal.Transceiver _delegate;
        private readonly string? _host;
        private readonly string? _adapterName;
        private readonly bool _incoming;
        private SslStream? _sslStream;
        private readonly int _verifyPeer;
        private bool _isConnected;
        private bool _authenticated;
        private IAsyncResult? _writeResult;
        private IAsyncResult? _readResult;
        private IceInternal.AsyncCallback? _readCallback;
        private IceInternal.AsyncCallback? _writeCallback;
        private int _maxSendPacketSize;
        private int _maxRecvPacketSize;
        private string? _cipher;
        private X509Certificate2[]? _certs;
        private bool _verified;
    }
}
