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
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Net;
    using System.Net.Security;
    using System.Net.Sockets;
    using System.Security.Authentication;
    using System.Security.Cryptography.X509Certificates;
    using System.Threading;

    sealed class TransceiverI : IceInternal.Transceiver
    {
        public bool restartable()
        {
            return false;
        }

        public bool initialize(AsyncCallback callback)
        {
            try
            {
                if(_state == StateNeedBeginConnect)
                {
                    Debug.Assert(callback != null);
                    Debug.Assert(_addr != null);

                    _state = StateNeedEndConnect;
                    _initializeResult = IceInternal.Network.doBeginConnectAsync(_fd, _addr, callback);

                    if(!_initializeResult.CompletedSynchronously)
                    {
                        //
                        // Return now if the I/O request needs an asynchronous callback.
                        //
                        return false;
                    }
                }

                if(_state == StateNeedEndConnect)
                {
                    Debug.Assert(_initializeResult != null);
                    IceInternal.Network.doEndConnectAsync(_initializeResult);
                    _state = StateNeedBeginAuthenticate;
                    _desc = IceInternal.Network.fdToString(_fd);
                    _initializeResult = null;
                }

                //
                // At this point the underlying TCP connection is established; now we need to
                // begin or end the SSL authentication process.
                //
                Debug.Assert(_state == StateNeedBeginAuthenticate || _state == StateNeedEndAuthenticate);
                if(_state == StateNeedBeginAuthenticate)
                {
                    _state = StateNeedEndAuthenticate;

                    //
                    // Return now if authentication needs an asynchronous callback.
                    //
                    if(!beginAuthenticate(callback))
                    {
                        return false;
                    }

                    endAuthenticate();
                }
                else
                {
                    endAuthenticate();
                }

                return true;
            }
            catch(Ice.LocalException e)
            {
                if(_instance.networkTraceLevel() >= 2)
                {
                    string s = "failed to establish ssl connection\n" + IceInternal.Network.fdToString(_fd) + "\n" + e;
                    _logger.trace(_instance.networkTraceCategory(), s);
                }
                throw;
            }
        }

        public void close()
        {
            if(_instance.networkTraceLevel() >= 1)
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

        public IAsyncResult beginRead(IceInternal.Buffer buf, AsyncCallback callback, object state)
        {
            Debug.Assert(_fd != null);

            int packetSize = buf.b.remaining();
            if(_maxReceivePacketSize > 0 && packetSize > _maxReceivePacketSize)
            {
                packetSize = _maxReceivePacketSize;
            }

            try
            {
                return _stream.BeginRead(buf.b.rawBytes(), buf.b.position(), packetSize, callback, state);
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

        public void endRead(IceInternal.Buffer buf, IAsyncResult result)
        {
            Debug.Assert(_fd != null);

            try
            {
                int ret = _stream.EndRead(result);
                if(ret == 0)
                {
                    throw new Ice.ConnectionLostException();
                }

                Debug.Assert(ret > 0);

                if(_instance.networkTraceLevel() >= 3)
                {
                    string s = "received " + ret + " of " + buf.b.remaining() + " bytes via ssl\n" + ToString();
                    _logger.trace(_instance.networkTraceCategory(), s);
                }

                if(_stats != null)
                {
                    _stats.bytesReceived(type(), ret);
                }

                buf.b.position(buf.b.position() + ret);
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

                Exception e = ex.InnerException;
                if(e != null && e is SocketException && IceInternal.Network.operationAborted((SocketException)e))
                {
                    throw new IceInternal.ReadAbortedException(ex);
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

        public IAsyncResult beginWrite(IceInternal.Buffer buf, AsyncCallback callback, object state)
        {
            Debug.Assert(_fd != null);

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
                IAsyncResult result = _stream.BeginWrite(buf.b.rawBytes(), buf.b.position(), packetSize, callback, 
                                                         state);
                _lastWriteSent = packetSize;
                buf.b.position(buf.b.position() + packetSize);
                return result;
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

        public void endWrite(IceInternal.Buffer buf, IAsyncResult result)
        {
            Debug.Assert(_fd != null);

            try
            {
                _stream.EndWrite(result);

                if(_instance.networkTraceLevel() >= 3)
                {
                    string s = "sent " + _lastWriteSent + " of " + (buf.b.remaining() + _lastWriteSent) +
                        " bytes via ssl\n" + ToString();
                    _logger.trace(_instance.networkTraceCategory(), s);
                }

                if(_stats != null)
                {
                    _stats.bytesSent(type(), _lastWriteSent);
                }
                _lastWriteSent = 0;
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

        public void checkSendSize(IceInternal.Buffer buf, int messageSizeMax)
        {
            if(buf.size() > messageSizeMax)
            {
                throw new Ice.MemoryLimitException();
            }
        }

        public override string ToString()
        {
            return _desc;
        }

        public ConnectionInfo getConnectionInfo()
        {
            return _info;
        }

        //
        // Only for use by ConnectorI, AcceptorI.
        //
        internal TransceiverI(Instance instance, Socket fd, IPEndPoint addr, bool connected, string host,
                              string adapterName)
        {
            _instance = instance;
            _fd = fd;
            _addr = addr;
            _host = host;
            _adapterName = adapterName;
            _stream = null;
            _info = null;
            _logger = instance.communicator().getLogger();
            _stats = instance.communicator().getStats();
            _desc = connected ? IceInternal.Network.fdToString(_fd) : "<not connected>";
            _state = connected ? StateNeedBeginAuthenticate : StateNeedBeginConnect;

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

        private bool beginAuthenticate(AsyncCallback callback)
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
                    _initializeResult = _stream.BeginAuthenticateAsClient(_host, _instance.certs(),
                                                                          _instance.protocols(),
                                                                          _instance.checkCRL() > 0,
                                                                          callback, null);
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

                    _initializeResult = _stream.BeginAuthenticateAsServer(cert, _verifyPeer > 1, _instance.protocols(),
                                                                          _instance.checkCRL() > 0, callback, null);
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

            Debug.Assert(_initializeResult != null);
            return _initializeResult.CompletedSynchronously;
        }

        private void endAuthenticate()
        {
            Debug.Assert(_initializeResult != null);

            try
            {
                if(_adapterName == null)
                {
                    _stream.EndAuthenticateAsClient(_initializeResult);
                }
                else
                {
                    _stream.EndAuthenticateAsServer(_initializeResult);
                }

                _state = StateAuthenticated;
                _initializeResult = null;

                _info = Util.populateConnectionInfo(_stream, _fd, _chain, _adapterName, _adapterName != null);
                _instance.verifyPeer(_info, _fd, _adapterName != null);

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
                if(_adapterName == null)
                {
                    if(!_instance.checkCertName())
                    {
                        errors ^= (int)SslPolicyErrors.RemoteCertificateNameMismatch;
                        message = message + "\nremote certificate name mismatch (ignored)";
                    }
                    else
                    {
                        if(_instance.securityTraceLevel() >= 1)
                        {
                            _logger.trace(_instance.securityTraceCategory(),
                                          "SSL certificate validation failed - remote certificate name mismatch");
                        }
                        return false;
                    }
                }
                else
                {
                    //
                    // This condition is not expected in a server.
                    //
                    Debug.Assert(false);
                }
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
                        if((certificate.Subject == certificate.Issuer) &&
                           (status.Status == X509ChainStatusFlags.UntrustedRoot))
                        {
                            //
                            // Untrusted root for self-signed certificate is OK.
                            //
                            --errorCount;
                        }
                        else if(status.Status == X509ChainStatusFlags.Revoked)
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

        private Instance _instance;
        private Socket _fd;
        private IPEndPoint _addr;
        private string _host;
        private string _adapterName;
        private SslStream _stream;
        private ConnectionInfo _info;
        private Ice.Logger _logger;
        private Ice.Stats _stats;
        private string _desc;
        private int _verifyPeer;
        private int _maxSendPacketSize;
        private int _maxReceivePacketSize;
        private int _lastWriteSent;
        private int _state;
        private IAsyncResult _initializeResult;
        private X509Certificate2[] _chain;

        private const int StateNeedBeginConnect = 0;
        private const int StateNeedEndConnect = 1;
        private const int StateNeedBeginAuthenticate = 2;
        private const int StateNeedEndAuthenticate = 3;
        private const int StateAuthenticated = 4;
    }
}
