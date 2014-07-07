// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

    sealed class TransceiverI : IceInternal.Transceiver
    {
        public int initialize()
        {
            try
            {
                if(_state == StateNeedConnect)
                {
                    Debug.Assert(_addr != null);
                    _state = StateConnectPending;
                    return IceInternal.SocketOperation.Connect;
                }
                else if(_state == StateConnectPending)
                {
                    IceInternal.Network.doFinishConnectAsync(_fd, _writeResult);
                    _writeResult = null;

                    _desc = IceInternal.Network.fdToString(_fd, _proxy, _addr);
                    if(_proxy != null)
                    {
                        _state = StateProxyConnectRequest; // Send proxy connect request
                        return IceInternal.SocketOperation.Write;
                    }

                    _state = StateAuthenticatePending;
                    return IceInternal.SocketOperation.Connect;
                }
                else if(_state == StateProxyConnectRequest)
                {
                    _state = StateProxyConnectRequestPending; // Wait for proxy response
                    return IceInternal.SocketOperation.Read;
                }
                else if(_state == StateProxyConnectRequestPending)
                {
                    _state = StateAuthenticatePending;
                    return IceInternal.SocketOperation.Connect;
                }
                else if(_state == StateNeedAuthenticate)
                {
                    _state = StateAuthenticatePending;
                    return IceInternal.SocketOperation.Connect;
                }
                else if(_state <= StateAuthenticatePending)
                {
                    endAuthenticate();
                    _writeResult = null;
                    _state = StateConnected;
                }
                return IceInternal.SocketOperation.None;
            }
            catch(Ice.LocalException e)
            {
                if(_instance.networkTraceLevel() >= 2)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("failed to establish ssl connection\n");
                    s.Append(IceInternal.Network.fdToString(_fd, _proxy, _addr));
                    s.Append("\n");
                    s.Append(e.ToString());
                    _logger.trace(_instance.networkTraceCategory(), s.ToString());
                }
                throw;
            }
        }

        public void close()
        {
            if(_state == StateConnected && _instance.networkTraceLevel() >= 1)
            {
                string s = "closing ssl connection\n" + ToString();
                _logger.trace(_instance.networkTraceCategory(), s);
            }

            Debug.Assert(_fd != null);
            try
            {
                if(_stream != null)
                {
                    //
                    // Closing the stream also closes the socket.
                    //
                    _stream.Close();
                }
                else
                {
                    _fd.Close();
                }
            }
            catch(IOException ex)
            {
                throw new Ice.SocketException(ex);
            }
            finally
            {
                _fd = null;
                _stream = null;
            }
        }

        public void destroy()
        {
            // Nothing to do.
        }

        public bool write(IceInternal.Buffer buf)
        {
            Debug.Assert(_fd != null);
            return false; // Caller will use async write.
        }

        public bool read(IceInternal.Buffer buf)
        {
            Debug.Assert(_fd != null);
            return false; // Caller will use async read.
        }

        public bool startRead(IceInternal.Buffer buf, IceInternal.AsyncCallback callback, object state)
        {
            Debug.Assert(_fd != null);

            if(_state == StateProxyConnectRequestPending)
            {
                _proxy.beginReadConnectRequestResponse(buf);
            }

            int packetSize = buf.b.remaining();
            if(_maxReceivePacketSize > 0 && packetSize > _maxReceivePacketSize)
            {
                packetSize = _maxReceivePacketSize;
            }

            try
            {
                _readCallback = callback;
                if(_stream != null)
                {
                    _readResult = _stream.BeginRead(buf.b.rawBytes(), buf.b.position(), packetSize, readCompleted,
                                                    state);
                }
                else
                {
                    _readResult = _fd.BeginReceive(buf.b.rawBytes(), buf.b.position(), packetSize, SocketFlags.None,
                                                   readCompleted, state);
                }
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
            catch(Ice.LocalException)
            {
                throw;
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
            if(_fd == null) // Transceiver was closed
            {
                _readResult = null;
                return;
            }

            Debug.Assert(_fd != null && _readResult != null);

            try
            {
                int ret = _stream != null ? _stream.EndRead(_readResult) : _fd.EndReceive(_readResult);
                _readResult = null;
                if(ret == 0)
                {
                    throw new Ice.ConnectionLostException();
                }

                Debug.Assert(ret > 0);

                if(_instance.networkTraceLevel() >= 3)
                {
                    int packetSize = buf.b.remaining();
                    if(_maxReceivePacketSize > 0 && packetSize > _maxReceivePacketSize)
                    {
                        packetSize = _maxReceivePacketSize;
                    }
                    string s = "received " + ret + " of " + packetSize + " bytes via ssl\n" + ToString();
                    _logger.trace(_instance.networkTraceCategory(), s);
                }

                if(_stats != null)
                {
#pragma warning disable 618
                    _stats.bytesReceived(type(), ret);
#pragma warning restore 618
                }

                buf.b.position(buf.b.position() + ret);

                if(_state == StateProxyConnectRequestPending)
                {
                    _proxy.endReadConnectRequestResponse(buf);
                }
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
            catch(Ice.LocalException)
            {
                throw;
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
            Debug.Assert(_fd != null);

            if(_state < StateConnected)
            {
                completed = false;
                if(_state == StateConnectPending)
                {
                    try
                    {
                        EndPoint addr = _proxy != null ? _proxy.getAddress() : _addr;
                        _writeResult = IceInternal.Network.doConnectAsync(_fd, addr, callback, state);
                        return _writeResult.CompletedSynchronously;
                    }
                    catch(Exception ex)
                    {
                        throw new Ice.SocketException(ex);
                    }
                }
                else if(_state == StateProxyConnectRequest)
                {
                    _proxy.beginWriteConnectRequest(_addr, buf);
                }
                else if(_state == StateAuthenticatePending)
                {
                    return beginAuthenticate(callback, state);
                }
            }

            //
            // We limit the packet size for beingWrite to ensure connection timeouts are based
            // on a fixed packet size.
            //
            int packetSize = buf.b.remaining();
            if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize)
            {
                packetSize = _maxSendPacketSize;
            }

            try
            {
                _writeCallback = callback;
                if(_stream != null)
                {
                    _writeResult = _stream.BeginWrite(buf.b.rawBytes(), buf.b.position(), packetSize, writeCompleted,
                                                      state);
                }
                else
                {
                    _writeResult = _fd.BeginSend(buf.b.rawBytes(), buf.b.position(), packetSize, SocketFlags.None,
                                                 writeCompleted, state);
                }
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
            catch(Ice.LocalException)
            {
                throw;
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
            if(_fd == null) // Transceiver was closed
            {
                if(buf.size() - buf.b.position() < _maxSendPacketSize)
                {
                    buf.b.position(buf.size()); // Assume all the data was sent for at-most-once semantics.
                }
                _writeResult = null;
                return;
            }

            if(_state < StateConnected && _state != StateProxyConnectRequest)
            {
                return;
            }

            Debug.Assert(_fd != null && _writeResult != null);

            try
            {
                if(_stream != null)
                {
                    _stream.EndWrite(_writeResult);
                }
                else
                {
                    _fd.EndSend(_writeResult);
                }
                _writeResult = null;

                int packetSize = buf.b.remaining();
                if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize)
                {
                    packetSize = _maxSendPacketSize;
                }

                if(_instance.networkTraceLevel() >= 3)
                {
                    string s = "sent " + packetSize + " of " + packetSize + " bytes via ssl\n" + ToString();
                    _logger.trace(_instance.networkTraceCategory(), s);
                }

                if(_stats != null)
                {
#pragma warning disable 618
                    _stats.bytesSent(type(), packetSize);
#pragma warning restore 618
                }

                buf.b.position(buf.b.position() + packetSize);

                if(_state == StateProxyConnectRequest)
                {
                    _proxy.endWriteConnectRequest(buf);
                }
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
            catch(Ice.LocalException)
            {
                throw;
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

        public string type()
        {
            return "ssl";
        }

        public Ice.ConnectionInfo getInfo()
        {
            return getNativeConnectionInfo();
        }

        public void checkSendSize(IceInternal.Buffer buf, int messageSizeMax)
        {
            if(buf.size() > messageSizeMax)
            {
                IceInternal.Ex.throwMemoryLimitException(buf.size(), messageSizeMax);
            }
        }

        public override string ToString()
        {
            return _desc;
        }

        //
        // Only for use by ConnectorI, AcceptorI.
        //
        internal TransceiverI(Instance instance, Socket fd, string host, bool connected,
                              bool incoming, string adapterName, IPEndPoint addr, IceInternal.NetworkProxy proxy)
        {
            _instance = instance;
            _fd = fd;
            _host = host;
            _incoming = incoming;
            _adapterName = adapterName;
            _addr = addr;
            _proxy = proxy;
            _stream = null;
            _logger = instance.communicator().getLogger();
            _stats = instance.communicator().getStats();
            _desc = connected ? IceInternal.Network.fdToString(_fd, _proxy, _addr) : "<not connected>";
            _state = connected ? StateNeedAuthenticate : StateNeedConnect;

            _maxSendPacketSize = IceInternal.Network.getSendBufferSize(fd);
            if(_maxSendPacketSize < 512)
            {
                _maxSendPacketSize = 0;
            }

            _maxReceivePacketSize = IceInternal.Network.getRecvBufferSize(fd);
            if(_maxReceivePacketSize < 512)
            {
                _maxReceivePacketSize = 0;
            }

            if(_adapterName != null)
            {
                //
                // Determine whether a certificate is required from the peer.
                //
                _verifyPeer =
                    _instance.communicator().getProperties().getPropertyAsIntWithDefault("IceSSL.VerifyPeer", 2);
            }
            else
            {
                _verifyPeer = 0;
            }
        }


        private NativeConnectionInfo getNativeConnectionInfo()
        {
            IceSSL.NativeConnectionInfo info = new IceSSL.NativeConnectionInfo();
            if(_fd != null)
            {
                IPEndPoint localEndpoint = (IPEndPoint)IceInternal.Network.getLocalAddress(_fd);
                info.localAddress = localEndpoint.Address.ToString();
                info.localPort = localEndpoint.Port;
                IPEndPoint remoteEndpoint = (IPEndPoint)IceInternal.Network.getRemoteAddress(_fd);
                if(remoteEndpoint != null)
                {
                    info.remoteAddress = remoteEndpoint.Address.ToString();
                    info.remotePort = remoteEndpoint.Port;
                }
            }
            if(_stream != null)
            {
                info.cipher = _stream.CipherAlgorithm.ToString();
                info.nativeCerts = _chain;
                List<string> certs = new List<string>();
                if(info.nativeCerts != null)
                {
                    foreach(X509Certificate2 cert in info.nativeCerts)
                    {
                        StringBuilder s = new StringBuilder();
                        s.Append("-----BEGIN CERTIFICATE-----\n");
                        s.Append(Convert.ToBase64String(cert.Export(X509ContentType.Cert)));
                        s.Append("\n-----END CERTIFICATE-----");
                        certs.Add(s.ToString());
                    }
                }
                info.certs = certs.ToArray();
            }
            info.adapterName = _adapterName;
            info.incoming = _adapterName != null;
            return info;
        }

        private bool beginAuthenticate(IceInternal.AsyncCallback callback, object state)
        {
            NetworkStream ns = new NetworkStream(_fd, true);
            _stream = new SslStream(ns, false, new RemoteCertificateValidationCallback(validationCallback), null);

            try
            {
                if(_adapterName == null)
                {
                    //
                    // Client authentication.
                    //
                    _writeResult = _stream.BeginAuthenticateAsClient(_host, _instance.certs(),
                                                                     _instance.protocols(),
                                                                     _instance.checkCRL() > 0,
                                                                     delegate(IAsyncResult result)
                                                                     {
                                                                         if(!result.CompletedSynchronously)
                                                                         {
                                                                             callback(result.AsyncState);
                                                                         }
                                                                     }, state);
                }
                else
                {
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

                    _writeResult = _stream.BeginAuthenticateAsServer(cert, _verifyPeer > 1, _instance.protocols(),
                                                                     _instance.checkCRL() > 0,
                                                                     delegate(IAsyncResult result)
                                                                     {
                                                                         if(!result.CompletedSynchronously)
                                                                         {
                                                                             callback(result.AsyncState);
                                                                         }
                                                                     }, state);
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
            catch(AuthenticationException ex)
            {
                Ice.SecurityException e = new Ice.SecurityException(ex);
                e.reason = ex.Message;
                throw e;
            }
            catch(Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }

            Debug.Assert(_writeResult != null);
            return _writeResult.CompletedSynchronously;
        }

        private void endAuthenticate()
        {
            Debug.Assert(_writeResult != null);

            try
            {
                if(_adapterName == null)
                {
                    _stream.EndAuthenticateAsClient(_writeResult);
                }
                else
                {
                    _stream.EndAuthenticateAsServer(_writeResult);
                }

                _instance.verifyPeer(getNativeConnectionInfo(), _fd, _host);

                if(_instance.networkTraceLevel() >= 1)
                {
                    string s;
                    if(_adapterName == null)
                    {
                        s = "ssl connection established\n" + _desc;
                    }
                    else
                    {
                        s = "accepted ssl connection\n" + _desc;
                    }
                    _logger.trace(_instance.networkTraceCategory(), s);
                }

                if(_instance.securityTraceLevel() >= 1)
                {
                    _instance.traceStream(_stream, _desc);
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
            catch(AuthenticationException ex)
            {
                Ice.SecurityException e = new Ice.SecurityException(ex);
                e.reason = ex.Message;
                throw e;
            }
            catch(Ice.LocalException)
            {
                throw;
            }
            catch(Exception ex)
            {
                throw new Ice.SyscallException(ex);
            }
        }

        private bool validationCallback(object sender, X509Certificate certificate, X509Chain chain,
                                        SslPolicyErrors sslPolicyErrors)
        {
            //
            // The certificate chain is not available via SslStream, and it is destroyed
            // after this callback returns, so we keep a reference to each of the
            // certificates.
            //
            if(chain != null)
            {
                _chain = new X509Certificate2[chain.ChainElements.Count];
                int i = 0;
                foreach(X509ChainElement e in chain.ChainElements)
                {
                    _chain[i++] = e.Certificate;
                }
            }

            string message = "";
            int errors = (int)sslPolicyErrors;
            if((errors & (int)SslPolicyErrors.RemoteCertificateNotAvailable) > 0)
            {
                //
                // The RemoteCertificateNotAvailable case does not appear to be possible
                // for an outgoing connection. Since .NET requires an authenticated
                // connection, the remote peer closes the socket if it does not have a
                // certificate to provide.
                //

                if(_adapterName != null)
                {
                    if(_verifyPeer > 1)
                    {
                        if(_instance.securityTraceLevel() >= 1)
                        {
                            _logger.trace(_instance.securityTraceCategory(),
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

            if((errors & (int)SslPolicyErrors.RemoteCertificateChainErrors) > 0)
            {
                if(chain == null && IceInternal.AssemblyUtil.platform_ != IceInternal.AssemblyUtil.Platform.Windows)
                {
                    //
                    // Mono doesn't supply the certificate chain.
                    //
                    errors ^= (int)SslPolicyErrors.RemoteCertificateChainErrors;
                }
                else if(chain != null && chain.ChainStatus != null)
                {
                    int errorCount = chain.ChainStatus.Length;
                    foreach(X509ChainStatus status in chain.ChainStatus)
                    {
                        if(status.Status == X509ChainStatusFlags.Revoked)
                        {
                            if(_instance.checkCRL() > 0)
                            {
                                message = message + "\ncertificate revoked";
                            }
                            else
                            {
                                message = message + "\ncertificate revoked (ignored)";
                                --errorCount;
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
                            }
                            else
                            {
                                message = message + "\ncertificate revocation status unknown (ignored)";
                                --errorCount;
                            }
                        }
                        else if(status.Status == X509ChainStatusFlags.NoError)
                        {
                            --errorCount;
                        }
                        else
                        {
                            message = message + "\ncertificate chain error: " + status.Status.ToString();
                        }
                    }

                    if(errorCount == 0)
                    {
                        errors ^= (int)SslPolicyErrors.RemoteCertificateChainErrors;
                    }
                }
            }

            if(errors > 0)
            {
                if(_instance.securityTraceLevel() >= 1)
                {
                    if(message.Length > 0)
                    {
                        _logger.trace(_instance.securityTraceCategory(), "SSL certificate validation failed:" +
                                      message);
                    }
                    else
                    {
                        _logger.trace(_instance.securityTraceCategory(), "SSL certificate validation failed");
                    }
                }
                return false;
            }
            else if(message.Length > 0 && _instance.securityTraceLevel() >= 1)
            {
                _logger.trace(_instance.securityTraceCategory(), "SSL certificate validation status:" + message);
            }

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
        private Socket _fd;
        private string _host;
        private bool _incoming;
        private string _adapterName;
        private IPEndPoint _addr;
        private IceInternal.NetworkProxy _proxy;
        private SslStream _stream;
        private Ice.Logger _logger;
        private Ice.Stats _stats;
        private string _desc;
        private int _verifyPeer;
        private int _maxSendPacketSize;
        private int _maxReceivePacketSize;
        private int _state;
        private IAsyncResult _writeResult;
        private IAsyncResult _readResult;
        private IceInternal.AsyncCallback _readCallback;
        private IceInternal.AsyncCallback _writeCallback;
        private X509Certificate2[] _chain;

        private const int StateNeedConnect = 0;
        private const int StateConnectPending = 1;
        private const int StateProxyConnectRequest = 2;
        private const int StateProxyConnectRequestPending = 3;
        private const int StateNeedAuthenticate = 4;
        private const int StateAuthenticatePending = 5;
        private const int StateConnected = 6;
    }
}
