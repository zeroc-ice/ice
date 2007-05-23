// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    using System.Net.Security;
    using System.Net.Sockets;
    using System.Security.Authentication;
    using System.Security.Cryptography.X509Certificates;
    using System.Threading;

    sealed class TransceiverI : IceInternal.Transceiver
    {
        public Socket fd()
        {
            Debug.Assert(fd_ != null);
            return fd_;
        }

        public void close()
        {
            if(instance_.networkTraceLevel() >= 1)
            {
                string s = "closing ssl connection\n" + ToString();
                logger_.trace(instance_.networkTraceCategory(), s);
            }

            lock(this)
            {
                Debug.Assert(fd_ != null);
                try
                {
                    if(stream_ != null)
                    {
                        //
                        // Closing the stream also closes the socket.
                        //
                        stream_.Close();
                    }
                    else
                    {
                        fd_.Close();
                    }
                }
                catch(IOException ex)
                {
                    throw new Ice.SocketException(ex);
                }
                finally
                {
                    fd_ = null;
                    stream_ = null;
                }
            }
        }

        public void shutdownWrite()
        {
            if(instance_.networkTraceLevel() >= 2)
            {
                string s = "shutting down ssl connection for writing\n" + ToString();
                logger_.trace(instance_.networkTraceCategory(), s);
            }

            Debug.Assert(fd_ != null);
            try
            {
                fd_.Shutdown(SocketShutdown.Send);
            }
            catch(SocketException ex)
            {
                if(IceInternal.Network.notConnected(ex))
                {
                    return;
                }
                throw new Ice.SocketException(ex);
            }
        }

        public void shutdownReadWrite()
        {
            if(instance_.networkTraceLevel() >= 2)
            {
                string s = "shutting down ssl connection for reading and writing\n" + ToString();
                logger_.trace(instance_.networkTraceCategory(), s);
            }

            Debug.Assert(fd_ != null);
            try
            {
                fd_.Shutdown(SocketShutdown.Both);
            }
            catch(SocketException ex)
            {
                if(IceInternal.Network.notConnected(ex))
                {
                    return;
                }
                throw new Ice.SocketException(ex);
            }
        }

        public void write(IceInternal.BasicStream stream, int timeout)
        {
            Debug.Assert(fd_ != null);

            IceInternal.ByteBuffer buf = stream.prepareWrite();
            int remaining = buf.remaining();
            int packetSize = remaining;
            if(maxPacketSize_ > 0 && packetSize > maxPacketSize_)
            {
                packetSize = maxPacketSize_;
            }

            try
            {
                while(remaining > 0)
                {
                    if(timeout == -1)
                    {
                        stream_.Write(buf.rawBytes(), buf.position(), packetSize);
                    }
                    else
                    {
                        //
                        // We have to use an asynchronous write to support a timeout.
                        //
                        IAsyncResult ar = stream_.BeginWrite(buf.rawBytes(), buf.position(), packetSize, null, null);
                        if(!ar.AsyncWaitHandle.WaitOne(timeout, false))
                        {
                            throw new Ice.TimeoutException();
                        }
                        stream_.EndWrite(ar);
                    }
                    if(instance_.networkTraceLevel() >= 3)
                    {
                        string s = "sent " + packetSize + " of " + packetSize + " bytes via ssl\n" + ToString();
                        logger_.trace(instance_.networkTraceCategory(), s);
                    }
                    if(stats_ != null)
                    {
                        stats_.bytesSent(type(), packetSize);
                    }
                    remaining -= packetSize;
                    buf.position(buf.position() + packetSize);
                    if(remaining < packetSize)
                    {
                        packetSize = remaining;
                    }
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
            catch(SocketException ex) // TODO: Necessary?
            {
                if(IceInternal.Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                if(IceInternal.Network.wouldBlock(ex))
                {
                    throw new Ice.TimeoutException();
                }
                throw new Ice.SocketException(ex);
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

        public void read(IceInternal.BasicStream stream, int timeout)
        {
            Debug.Assert(fd_ != null);

            IceInternal.ByteBuffer buf = stream.prepareRead();    
            int remaining = buf.remaining();
            int position = buf.position();

            try
            {
                int ret = 0;
                while(remaining > 0)
                {
                    if(timeout == -1)
                    {
                        ret = stream_.Read(buf.rawBytes(), position, remaining);
                        if(ret == 0)
                        {
                            //
                            // Try to read again; if zero is returned, the connection is lost.
                            //
                            ret = stream_.Read(buf.rawBytes(), position, remaining);
                            if(ret == 0)
                            {
                                throw new Ice.ConnectionLostException();
                            }
                        }
                    }
                    else
                    {
                        //
                        // We have to use an asynchronous read to support a timeout.
                        //
                        IAsyncResult ar = stream_.BeginRead(buf.rawBytes(), position, remaining, null, null);
                        if(!ar.AsyncWaitHandle.WaitOne(timeout, false))
                        {
                            throw new Ice.TimeoutException();
                        }
                        ret = stream_.EndRead(ar);
                    }
                    if(instance_.networkTraceLevel() >= 3)
                    {
                        string s = "received " + ret + " of " + remaining + " bytes via ssl\n" + ToString();
                        logger_.trace(instance_.networkTraceCategory(), s);
                    }    
                    if(stats_ != null)
                    {
                        stats_.bytesReceived(type(), ret);
                    }
                    remaining -= ret;
                    buf.position(position += ret);
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
            catch(SocketException ex) // TODO: Necessary?
            {
                if(IceInternal.Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                if(IceInternal.Network.wouldBlock(ex))
                {
                    throw new Ice.TimeoutException();
                }
                throw new Ice.SocketException(ex);
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

        public string type()
        {
            return "ssl";
        }

        public void initialize(int timeout)
        {
            if(stream_ == null)
            {
                try
                {
                    //
                    // Create an SslStream.
                    //
                    NetworkStream ns = new NetworkStream(fd_, true);
                    TransceiverValidationCallback cb = new TransceiverValidationCallback(this);
                    stream_ = new SslStream(ns, false, new RemoteCertificateValidationCallback(cb.validate), null);

                    //
                    // Get the certificate collection and select the first one.
                    //
                    X509Certificate2Collection certs = instance_.certs();
                    X509Certificate2 cert = null;
                    if(certs.Count > 0)
                    {
                        cert = certs[0];
                    }

                    //
                    // Start the validation process and wait for it to complete.
                    //
                    AuthInfo info = new AuthInfo();
                    info.stream = stream_;
                    info.done = false;
                    stream_.BeginAuthenticateAsServer(cert, verifyPeer_ > 1, instance_.protocols(),
                                                      instance_.checkCRL(), new AsyncCallback(authCallback), info);
                    lock(info)
                    {
                        if(!info.done)
                        {
                            if(!Monitor.Wait(info, timeout == -1 ? Timeout.Infinite : timeout))
                            {
                                throw new Ice.TimeoutException();
                            }
                        }
                        if(info.ex != null)
                        {
                            throw info.ex;
                        }
                    }

                    info_ = Util.populateConnectionInfo(stream_, fd_, cb.certs, adapterName_, true);
                    instance_.verifyPeer(info_, fd_, true);
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

                if(instance_.networkTraceLevel() >= 1)
                {
                    string s = "accepted ssl connection\n" + IceInternal.Network.fdToString(fd_);
                    logger_.trace(instance_.networkTraceCategory(), s);
                }

                if(instance_.securityTraceLevel() >= 1)
                {
                    instance_.traceStream(stream_, IceInternal.Network.fdToString(fd_));
                }
            }
        }

        public void checkSendSize(IceInternal.BasicStream stream, int messageSizeMax)
        {
            if(stream.size() > messageSizeMax)
            {
                throw new Ice.MemoryLimitException();
            }
        }

        public override string ToString()
        {
            return desc_;
        }

        public ConnectionInfo getConnectionInfo()
        {
            return info_;
        }

        //
        // Only for use by ConnectorI.
        //
        internal TransceiverI(Instance instance, Socket fd, SslStream stream, ConnectionInfo info)
        {
            instance_ = instance;
            fd_ = fd;
            stream_ = stream;
            info_ = info;
            logger_ = instance.communicator().getLogger();
            stats_ = instance.communicator().getStats();
            desc_ = IceInternal.Network.fdToString(fd_);
            verifyPeer_ = 0;

            maxPacketSize_ = 0;
            if(IceInternal.AssemblyUtil.platform_ == IceInternal.AssemblyUtil.Platform.Windows)
            {
		//
		// On Windows, limiting the buffer size is important to prevent
		// poor throughput performances when transfering large amount of
		// data. See Microsoft KB article KB823764.
		//
                maxPacketSize_ = IceInternal.Network.getSendBufferSize(fd) / 2;
                if(maxPacketSize_ < 512)
                {
                    maxPacketSize_ = 0;
                }
            }
        }

        //
        // Only for use by AcceptorI.
        //
        internal TransceiverI(Instance instance, Socket fd, string adapterName)
        {
            instance_ = instance;
            fd_ = fd;
            stream_ = null;
            info_ = null;
            adapterName_ = adapterName;
            logger_ = instance.communicator().getLogger();
            stats_ = instance.communicator().getStats();
            desc_ = IceInternal.Network.fdToString(fd_);
            
            maxPacketSize_ = 0;
            if(IceInternal.AssemblyUtil.platform_ == IceInternal.AssemblyUtil.Platform.Windows)
            {
		//
		// On Windows, limiting the buffer size is important to prevent
		// poor throughput performances when transfering large amount of
		// data. See Microsoft KB article KB823764.
		//
                maxPacketSize_ = IceInternal.Network.getSendBufferSize(fd) / 2;
                if(maxPacketSize_ < 512)
                {
                    maxPacketSize_ = 0;
                }
            }
            
            //
            // Determine whether a certificate is required from the peer.
            //
            verifyPeer_ = instance_.communicator().getProperties().getPropertyAsIntWithDefault("IceSSL.VerifyPeer", 2);
        }

#if DEBUG
        ~TransceiverI()
        {
            /*
            lock(this)
            {
                IceUtil.Assert.FinalizerAssert(fd_ == null);
            }
            */
        }
#endif

        internal bool validate(object sender, X509Certificate certificate, X509Chain chain,
                               SslPolicyErrors sslPolicyErrors)
        {
            string message = "";
            int errors = (int)sslPolicyErrors;
            if((errors & (int)SslPolicyErrors.RemoteCertificateNotAvailable) > 0)
            {
                if(verifyPeer_ > 1)
                {
                    if(instance_.securityTraceLevel() >= 1)
                    {
                        logger_.trace(instance_.securityTraceCategory(),
                                      "SSL certificate validation failed - client certificate not provided");
                    }
                    return false;
                }
                errors ^= (int)SslPolicyErrors.RemoteCertificateNotAvailable;
                message = message + "\nremote certificate not provided (ignored)";
            }

            if((errors & (int)SslPolicyErrors.RemoteCertificateNameMismatch) > 0)
            {
                //
                // This condition is not expected in a server.
                //
                Debug.Assert(false);
            }

            if(errors > 0)
            {
                if(instance_.securityTraceLevel() >= 1)
                {
                    logger_.trace(instance_.securityTraceCategory(), "SSL certificate validation failed");
                }
                return false;
            }

            return true;
        }

        private class AuthInfo
        {
            internal SslStream stream;
            volatile internal Exception ex;
            volatile internal bool done;
        }

        private static void authCallback(IAsyncResult ar)
        {
            AuthInfo info = (AuthInfo)ar.AsyncState;
            lock(info)
            {
                try
                {
                    info.stream.EndAuthenticateAsServer(ar);
                }
                catch(Exception ex)
                {
                    info.ex = ex;
                }
                finally
                {
                    info.done = true;
                    Monitor.Pulse(info);
                }
            }
        }

        private Instance instance_;
        private Socket fd_;
        private SslStream stream_;
        private ConnectionInfo info_;
        private string adapterName_;
        private Ice.Logger logger_;
        private Ice.Stats stats_;
        private string desc_;
        private int verifyPeer_;
        private int maxPacketSize_;
    }

    internal class TransceiverValidationCallback
    {
        internal TransceiverValidationCallback(TransceiverI transceiver)
        {
            transceiver_ = transceiver;
            certs = null;
        }

        internal bool validate(object sender, X509Certificate certificate, X509Chain chain,
                               SslPolicyErrors sslPolicyErrors)
        {
            //
            // The certificate chain is not available via SslStream, and it is destroyed
            // after this callback returns, so we keep a reference to each of the
            // certificates.
            //
            if(chain != null)
            {
                certs = new X509Certificate2[chain.ChainElements.Count];
                int i = 0;
                foreach(X509ChainElement e in chain.ChainElements)
                {
                    certs[i++] = e.Certificate;
                }
            }
            return transceiver_.validate(sender, certificate, chain, sslPolicyErrors);
        }

        private TransceiverI transceiver_;
        internal X509Certificate2[] certs;
    }
}
