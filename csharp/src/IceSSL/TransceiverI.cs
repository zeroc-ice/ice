// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Collections.Generic;
    using System.IO;
    using System.Net;
    using System.Net.Security;
    using System.Net.Sockets;
    using System.Security.Authentication;
    using System.Security.Cryptography.X509Certificates;
    using System.Threading;
    using System.Text;

    sealed class TransceiverI : IceInternal.Transceiver, IceInternal.WSTransceiverDelegate
    {
        public Socket fd()
        {
            return _stream.fd();
        }

        public int initialize(IceInternal.Buffer readBuffer, IceInternal.Buffer writeBuffer, ref bool hasMoreData)
        {
            int status = _stream.connect(readBuffer, writeBuffer, ref hasMoreData);
            if(status != IceInternal.SocketOperation.None)
            {
                return status;
            }

            _stream.setBlock(true); // SSL requires a blocking socket

            if(_sslStream == null)
            {
                NetworkStream ns = new NetworkStream(_stream.fd(), false);
                _sslStream = new SslStream(ns, false, new RemoteCertificateValidationCallback(validationCallback),
                                                      new LocalCertificateSelectionCallback(selectCertificate));
                return IceInternal.SocketOperation.Connect;
            }

            Debug.Assert(_sslStream.IsAuthenticated);
            _authenticated = true;
            _instance.verifyPeer((NativeConnectionInfo)getInfo(), _stream.fd(), _host);

            if(_instance.securityTraceLevel() >= 1)
            {
                _instance.traceStream(_sslStream, _stream.ToString());
            }
            return IceInternal.SocketOperation.None;
        }

        public int closing(bool initiator, Ice.LocalException ex)
        {
            // If we are initiating the connection closure, wait for the peer
            // to close the TCP/IP connection. Otherwise, close immediately.
            return initiator ? IceInternal.SocketOperation.Read : IceInternal.SocketOperation.None;
        }

        public void close()
        {
            if(_sslStream != null)
            {
                _sslStream.Close(); // Closing the stream also closes the socket.
                _sslStream = null;
            }

            _stream.close();
        }

        public IceInternal.EndpointI bind()
        {
            Debug.Assert(false);
            return null;
        }

        public void destroy()
        {
            _stream.destroy();
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
            if(!_stream.isConnected())
            {
                return _stream.startRead(buf, callback, state);
            }

            Debug.Assert(_sslStream != null && _sslStream.IsAuthenticated);

            int packetSz = _stream.getRecvPacketSize(buf.b.remaining());
            try
            {
                _readCallback = callback;
                _readResult = _sslStream.BeginRead(buf.b.rawBytes(), buf.b.position(), packetSz, readCompleted, state);
                return _readResult.CompletedSynchronously;
            }
            catch(IOException ex)
            {
                if(IceInternal.Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                if(IceInternal.Network.timeout(ex))
                {
                    throw new Ice.TimeoutException();
                }
                throw new Ice.SocketException(ex);
            }
            catch(ObjectDisposedException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
            catch(Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }
        }

        public void finishRead(IceInternal.Buffer buf)
        {
            if(!_stream.isConnected())
            {
                _stream.finishRead(buf);
                return;
            }
            else if(_sslStream == null) // Transceiver was closed
            {
                _readResult = null;
                return;
            }

            Debug.Assert(_readResult != null);
            try
            {
                int ret = _sslStream.EndRead(_readResult);
                _readResult = null;

                if(ret == 0)
                {
                    throw new Ice.ConnectionLostException();
                }
                Debug.Assert(ret > 0);
                buf.b.position(buf.b.position() + ret);
            }
            catch(Ice.LocalException)
            {
                throw;
            }
            catch(IOException ex)
            {
                if(IceInternal.Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                if(IceInternal.Network.timeout(ex))
                {
                    throw new Ice.TimeoutException();
                }
                throw new Ice.SocketException(ex);
            }
            catch(ObjectDisposedException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
            catch(Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }
        }

        public bool startWrite(IceInternal.Buffer buf, IceInternal.AsyncCallback callback, object state,
                               out bool completed)
        {
            if(!_stream.isConnected())
            {
                return _stream.startWrite(buf, callback, state, out completed);
            }

            Debug.Assert(_sslStream != null);
            if(!_authenticated)
            {
                completed = false;
                return startAuthenticate(callback, state);
            }

            //
            // We limit the packet size for beingWrite to ensure connection timeouts are based
            // on a fixed packet size.
            //
            int packetSize = _stream.getSendPacketSize(buf.b.remaining());
            try
            {
                _writeCallback = callback;
                _writeResult = _sslStream.BeginWrite(buf.b.rawBytes(), buf.b.position(), packetSize, writeCompleted,
                                                     state);
                completed = packetSize == buf.b.remaining();
                return _writeResult.CompletedSynchronously;
            }
            catch(IOException ex)
            {
                if(IceInternal.Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                if(IceInternal.Network.timeout(ex))
                {
                    throw new Ice.TimeoutException();
                }
                throw new Ice.SocketException(ex);
            }
            catch(ObjectDisposedException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
            catch(Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }
        }

        public void finishWrite(IceInternal.Buffer buf)
        {
            if(!_stream.isConnected())
            {
                _stream.finishWrite(buf);
                return;
            }
            else if(_sslStream == null) // Transceiver was closed
            {
                if(_stream.getSendPacketSize(buf.b.remaining()) == buf.b.remaining()) // Sent last packet
                {
                    buf.b.position(buf.b.limit()); // Assume all the data was sent for at-most-once semantics.
                }
                _writeResult = null;
                return;
            }
            else if(!_authenticated)
            {
                finishAuthenticate();
                return;
            }

            Debug.Assert(_writeResult != null);
            int sent = _stream.getSendPacketSize(buf.b.remaining());
            try
            {
                _sslStream.EndWrite(_writeResult);
                _writeResult = null;
                buf.b.position(buf.b.position() + sent);
            }
            catch(IOException ex)
            {
                if(IceInternal.Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                if(IceInternal.Network.timeout(ex))
                {
                    throw new Ice.TimeoutException();
                }
                throw new Ice.SocketException(ex);
            }
            catch(ObjectDisposedException ex)
            {
                throw new Ice.ConnectionLostException(ex);
            }
            catch(Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }
        }

        public string protocol()
        {
            return _instance.protocol();
        }

        public Ice.ConnectionInfo getInfo()
        {
            NativeConnectionInfo info = new NativeConnectionInfo();
            info.nativeCerts = fillConnectionInfo(info);
            return info;
        }

        public Ice.ConnectionInfo getWSInfo(Dictionary<string, string> headers)
        {
            WSSNativeConnectionInfo info = new WSSNativeConnectionInfo();
            info.nativeCerts = fillConnectionInfo(info);
            info.headers = headers;
            return info;
        }

        public void checkSendSize(IceInternal.Buffer buf)
        {
        }

        public void setBufferSize(int rcvSize, int sndSize)
        {
            _stream.setBufferSize(rcvSize, sndSize);
        }

        public override string ToString()
        {
            return _stream.ToString();
        }

        public string toDetailedString()
        {
            return ToString();
        }

        //
        // Only for use by ConnectorI, AcceptorI.
        //
        internal TransceiverI(Instance instance, IceInternal.StreamSocket stream, string hostOrAdapterName, bool incoming)
        {
            _instance = instance;
            _stream = stream;
            _incoming = incoming;
            if(_incoming)
            {
                _adapterName = hostOrAdapterName;
            }
            else
            {
                _host = hostOrAdapterName;
            }
            _sslStream = null;

            _verifyPeer = _instance.properties().getPropertyAsIntWithDefault("IceSSL.VerifyPeer", 2);

            _chain = new X509Chain(_instance.engine().useMachineContext());

            if(_instance.checkCRL() == 0)
            {
                _chain.ChainPolicy.RevocationMode = X509RevocationMode.NoCheck;
            }

            X509Certificate2Collection caCerts = _instance.engine().caCerts();
            if(caCerts != null)
            {
#if !UNITY
                //
                // We need to set this flag to be able to use a certificate authority from the extra store.
                //
                _chain.ChainPolicy.VerificationFlags = X509VerificationFlags.AllowUnknownCertificateAuthority;
#endif
                foreach(X509Certificate2 cert in caCerts)
                {
                    _chain.ChainPolicy.ExtraStore.Add(cert);
                }
            }
        }

        private X509Certificate2[] fillConnectionInfo(ConnectionInfo info)
        {
            X509Certificate2[] nativeCerts = null;
            if(_stream.fd() != null)
            {
                IPEndPoint localEndpoint = (IPEndPoint)IceInternal.Network.getLocalAddress(_stream.fd());
                info.localAddress = localEndpoint.Address.ToString();
                info.localPort = localEndpoint.Port;
                IPEndPoint remoteEndpoint = (IPEndPoint)IceInternal.Network.getRemoteAddress(_stream.fd());
                if(remoteEndpoint != null)
                {
                    info.remoteAddress = remoteEndpoint.Address.ToString();
                    info.remotePort = remoteEndpoint.Port;
                }
                info.rcvSize = IceInternal.Network.getRecvBufferSize(_stream.fd());
                info.sndSize = IceInternal.Network.getSendBufferSize(_stream.fd());
            }
            if(_sslStream != null)
            {
#if UNITY
                info.cipher = "";
#else
                info.cipher = _sslStream.CipherAlgorithm.ToString();
                if(_chain.ChainElements != null && _chain.ChainElements.Count > 0)
                {
                    nativeCerts = new X509Certificate2[_chain.ChainElements.Count];
                    for(int i = 0; i < _chain.ChainElements.Count; ++i)
                    {
                        nativeCerts[i] = _chain.ChainElements[i].Certificate;
                    }
                }
#endif

                List<string> certs = new List<string>();
#if !UNITY
                if(nativeCerts != null)
                {
                    foreach(X509Certificate2 cert in nativeCerts)
                    {
                        StringBuilder s = new StringBuilder();
                        s.Append("-----BEGIN CERTIFICATE-----\n");
                        s.Append(Convert.ToBase64String(cert.Export(X509ContentType.Cert)));
                        s.Append("\n-----END CERTIFICATE-----");
                        certs.Add(s.ToString());
                    }
                }
#endif
                info.certs = certs.ToArray();
                info.verified = _verified;
            }
            info.adapterName = _adapterName;
            info.incoming = _incoming;
            return nativeCerts;
        }

        private bool startAuthenticate(IceInternal.AsyncCallback callback, object state)
        {
            try
            {
                _writeCallback = callback;
                if(!_incoming)
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
#if UNITY
                    throw new Ice.FeatureNotSupportedException("ssl server socket");
#else
                    //
                    // Server authentication.
                    //
                    // Get the certificate collection and select the first one.
                    //
                    X509Certificate2Collection certs = _instance.certs();
                    X509Certificate2 cert = null;
                    if(certs.Count > 0)
                    {
                        cert = certs[0];
                    }

                    _writeResult = _sslStream.BeginAuthenticateAsServer(cert,
                                                                        _verifyPeer > 0,
                                                                        _instance.protocols(),
                                                                        _instance.checkCRL() > 0,
                                                                        writeCompleted,
                                                                        state);
#endif
                }
            }
            catch(IOException ex)
            {
                if(IceInternal.Network.connectionLost(ex))
                {
                    //
                    // This situation occurs when connectToSelf is called; the "remote" end
                    // closes the socket immediately.
                    //
                    throw new Ice.ConnectionLostException();
                }
                throw new Ice.SocketException(ex);
            }
#if !UNITY
            catch(AuthenticationException ex)
            {
                Ice.SecurityException e = new Ice.SecurityException(ex);
                e.reason = ex.Message;
                throw e;
            }
#endif
            catch(Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }

            Debug.Assert(_writeResult != null);
            return _writeResult.CompletedSynchronously;
        }

        private void finishAuthenticate()
        {
            Debug.Assert(_writeResult != null);

            try
            {
                if(!_incoming)
                {
                    _sslStream.EndAuthenticateAsClient(_writeResult);
                }
                else
                {
                    _sslStream.EndAuthenticateAsServer(_writeResult);
                }
            }
            catch(IOException ex)
            {
                if(IceInternal.Network.connectionLost(ex))
                {
                    //
                    // This situation occurs when connectToSelf is called; the "remote" end
                    // closes the socket immediately.
                    //
                    throw new Ice.ConnectionLostException();
                }
                throw new Ice.SocketException(ex);
            }
#if !UNITY
            catch(AuthenticationException ex)
            {
                Ice.SecurityException e = new Ice.SecurityException(ex);
                e.reason = ex.Message;
                throw e;
            }
#endif
            catch(Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }
        }

        private X509Certificate selectCertificate(
            object sender,
            string targetHost,
            X509CertificateCollection certs,
            X509Certificate remoteCertificate,
            string[] acceptableIssuers)
        {
            if(certs == null || certs.Count == 0)
            {
                return null;
            }
            else if(certs.Count == 1)
            {
                return certs[0];
            }

            //
            // Use the first certificate that match the acceptable issuers.
            //
            if(acceptableIssuers != null && acceptableIssuers.Length > 0)
            {
                foreach(X509Certificate certificate in certs)
                {
                    if(Array.IndexOf(acceptableIssuers, certificate.Issuer) != -1)
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
#if !UNITY
            string message = "";
            int errors = (int)policyErrors;
            if(certificate != null)
            {
                _chain.Build(new X509Certificate2(certificate));
                if(_chain.ChainStatus != null && _chain.ChainStatus.Length > 0)
                {
                    errors = (int)SslPolicyErrors.RemoteCertificateChainErrors;
                }
                else if(_instance.engine().caCerts() != null)
                {
                    X509ChainElement e = _chain.ChainElements[_chain.ChainElements.Count - 1];
                    if(!_chain.ChainPolicy.ExtraStore.Contains(e.Certificate))
                    {
                        if(_verifyPeer > 0)
                        {
                            message = message + "\npuntrusted root certificate";
                        }
                        else
                        {
                            message = message + "\nuntrusted root certificate (ignored)";
                            _verified = false;
                        }
                        errors = (int)SslPolicyErrors.RemoteCertificateChainErrors;
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

            if((errors & (int)SslPolicyErrors.RemoteCertificateNotAvailable) > 0)
            {
                //
                // The RemoteCertificateNotAvailable case does not appear to be possible
                // for an outgoing connection. Since .NET requires an authenticated
                // connection, the remote peer closes the socket if it does not have a
                // certificate to provide.
                //

                if(_incoming)
                {
                    if(_verifyPeer > 1)
                    {
                        if(_instance.securityTraceLevel() >= 1)
                        {
                            _instance.logger().trace(_instance.securityTraceCategory(),
                                          "SSL certificate validation failed - client certificate not provided");
                        }
                        return false;
                    }
                    errors ^= (int)SslPolicyErrors.RemoteCertificateNotAvailable;
                    message = message + "\nremote certificate not provided (ignored)";
                }
            }

            if((errors & (int)SslPolicyErrors.RemoteCertificateNameMismatch) > 0)
            {
                //
                // Ignore this error here; we'll check the peer certificate in verifyPeer().
                //
                errors ^= (int)SslPolicyErrors.RemoteCertificateNameMismatch;
            }


            if((errors & (int)SslPolicyErrors.RemoteCertificateChainErrors) > 0 &&
                _chain.ChainStatus != null && _chain.ChainStatus.Length > 0)
            {
                int errorCount = 0;
                foreach(X509ChainStatus status in _chain.ChainStatus)
                {
                    if(status.Status == X509ChainStatusFlags.UntrustedRoot && _instance.engine().caCerts() != null)
                    {
                        //
                        // Untrusted root is OK when using our custom chain engine if
                        // the CA certificate is present in the chain policy extra store.
                        //
                        X509ChainElement e = _chain.ChainElements[_chain.ChainElements.Count - 1];
                        if(!_chain.ChainPolicy.ExtraStore.Contains(e.Certificate))
                        {
                            if(_verifyPeer > 0)
                            {
                                message = message + "\npuntrusted root certificate";
                                ++errorCount;
                            }
                            else
                            {
                                message = message + "\nuntrusted root certificate (ignored)";
                            }
                        }
                        else
                        {
                            _verified = true;
                        }
                    }
                    else if(status.Status == X509ChainStatusFlags.Revoked)
                    {
                        if(_instance.checkCRL() > 0)
                        {
                            message = message + "\ncertificate revoked";
                            ++errorCount;
                        }
                        else
                        {
                            message = message + "\ncertificate revoked (ignored)";
                        }
                    }
                    else if(status.Status == X509ChainStatusFlags.RevocationStatusUnknown)
                    {
                        //
                        // If a certificate's revocation status cannot be determined, the strictest
                        // policy is to reject the connection.
                        //
                        if(_instance.checkCRL() > 1)
                        {
                            message = message + "\ncertificate revocation status unknown";
                            ++errorCount;
                        }
                        else
                        {
                            message = message + "\ncertificate revocation status unknown (ignored)";
                        }
                    }
                    else if(status.Status == X509ChainStatusFlags.PartialChain)
                    {
                        if(_verifyPeer > 0)
                        {
                            message = message + "\npartial certificate chain";
                            ++errorCount;
                        }
                        else
                        {
                            message = message + "\npartial certificate chain (ignored)";
                        }
                    }
                    else if(status.Status != X509ChainStatusFlags.NoError)
                    {
                        message = message + "\ncertificate chain error: " + status.Status.ToString();
                        ++errorCount;
                    }
                }

                if(errorCount == 0)
                {
                    errors ^= (int)SslPolicyErrors.RemoteCertificateChainErrors;
                }
            }

            if(errors > 0)
            {
                if(_instance.securityTraceLevel() >= 1)
                {
                    if(message.Length > 0)
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
            else if(message.Length > 0 && _instance.securityTraceLevel() >= 1)
            {
                _instance.logger().trace(_instance.securityTraceCategory(),
                                         "SSL certificate validation status:" + message);
            }
#endif
            return true;
        }

        internal void readCompleted(IAsyncResult result)
        {
            if(!result.CompletedSynchronously)
            {
                _readCallback(result.AsyncState);
            }
        }

        internal void writeCompleted(IAsyncResult result)
        {
            if(!result.CompletedSynchronously)
            {
                _writeCallback(result.AsyncState);
            }
        }

        private Instance _instance;
        private IceInternal.StreamSocket _stream;
        private string _host = "";
        private string _adapterName = "";
        private bool _incoming;
        private SslStream _sslStream;
        private int _verifyPeer;
        private bool _authenticated;
        private IAsyncResult _writeResult;
        private IAsyncResult _readResult;
        private IceInternal.AsyncCallback _readCallback;
        private IceInternal.AsyncCallback _writeCallback;
        private X509Chain _chain;
        private bool _verified;
    }
}
