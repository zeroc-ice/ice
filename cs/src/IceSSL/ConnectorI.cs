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
    using System.Diagnostics;
    using System.IO;
    using System.Net;
    using System.Net.Security;
    using System.Net.Sockets;
    using System.Security.Authentication;
    using System.Security.Cryptography.X509Certificates;
    using System.Threading;

    sealed class ConnectorI : IceInternal.Connector, System.IComparable
    {
        internal const short TYPE = 2;

        public IceInternal.Transceiver connect(int timeout)
        {
            //
            // The plugin may not be fully initialized.
            //
            if(!instance_.initialized())
            {
                Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
                ex.reason = "IceSSL: plugin is not initialized";
                throw ex;
            }

            if(instance_.networkTraceLevel() >= 2)
            {
                string s = "trying to establish ssl connection to " + ToString();
                logger_.trace(instance_.networkTraceCategory(), s);
            }

            Socket fd = IceInternal.Network.createSocket(false);
            IceInternal.Network.setBlock(fd, true);
            IceInternal.Network.setTcpBufSize(fd, instance_.communicator().getProperties(), logger_);
            IceInternal.Network.doConnectAsync(fd, addr_, timeout);

            SslStream stream = null;
            ConnectionInfo connInfo = null;
            try
            {
                //
                // Create an SslStream.
                //
                NetworkStream ns = new NetworkStream(fd, true);
                ConnectorValidationCallback cb = new ConnectorValidationCallback(this);
                stream = new SslStream(ns, false, new RemoteCertificateValidationCallback(cb.validate), null);

                //
                // Start the validation process and wait for it to complete.
                //
                AuthInfo info = new AuthInfo();
                info.stream = stream;
                info.done = false;
                stream.BeginAuthenticateAsClient(host_, instance_.certs(), instance_.protocols(),
                                                 instance_.checkCRL(),
                                                 new AsyncCallback(authCallback), info);
                lock(info)
                {
                    if(!info.done)
                    {
                        if(!Monitor.Wait(info, timeout == -1 ? Timeout.Infinite : timeout))
                        {
                            throw new Ice.ConnectTimeoutException();
                        }
                    }
                    if(info.ex != null)
                    {
                        throw info.ex;
                    }
                }

                connInfo = Util.populateConnectionInfo(stream, fd, cb.certs, "", false);
                instance_.verifyPeer(connInfo, fd, false);
            }
            catch(Ice.LocalException ex)
            {
                if(stream != null)
                {
                    stream.Close();
                }
                else
                {
                    IceInternal.Network.closeSocketNoThrow(fd);
                }

                throw ex;
            }
            catch(IOException ex)
            {
                if(stream != null)
                {
                    stream.Close();
                }
                else
                {
                    IceInternal.Network.closeSocketNoThrow(fd);
                }

                if(IceInternal.Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                throw new Ice.SyscallException(ex);
            }
            catch(AuthenticationException ex)
            {
                if(stream != null)
                {
                    stream.Close();
                }
                else
                {
                    IceInternal.Network.closeSocketNoThrow(fd);
                }

                Ice.SecurityException e = new Ice.SecurityException(ex);
                e.reason = ex.Message;
                throw e;
            }
            catch(Exception ex)
            {
                if(stream != null)
                {
                    stream.Close();
                }
                else
                {
                    IceInternal.Network.closeSocketNoThrow(fd);
                }

                throw new Ice.SyscallException(ex);
            }

            if(instance_.networkTraceLevel() >= 1)
            {
                string s = "ssl connection established\n" + IceInternal.Network.fdToString(fd);
                logger_.trace(instance_.networkTraceCategory(), s);
            }

            if(instance_.securityTraceLevel() >= 1)
            {
                instance_.traceStream(stream, IceInternal.Network.fdToString(fd));
            }

            return new TransceiverI(instance_, fd, stream, connInfo);
        }

        public short type()
        {
            return TYPE;
        }

        public override string ToString()
        {
            return IceInternal.Network.addrToString(addr_);
        }

        //
        // Only for use by EndpointI.
        //
        internal ConnectorI(Instance instance, IPEndPoint addr, int timeout, string connectionId)
        {
            instance_ = instance;
            host_ = addr.Address.ToString();
            logger_ = instance.communicator().getLogger();
            addr_ = addr;
            timeout_ = timeout;
            connectionId_ = connectionId;

            hashCode_ = addr_.GetHashCode();
            hashCode_ = 5 * hashCode_ + timeout_;
            hashCode_ = 5 * hashCode_ + connectionId_.GetHashCode();
        }

        public override int GetHashCode()
        {
            return hashCode_;
        }

        //
        // Compare endpoints for sorting purposes
        //
        public override bool Equals(object obj)
        {
            return CompareTo(obj) == 0;
        }

        public int CompareTo(object obj)
        {
            ConnectorI p = null;

            try
            {
                p = (ConnectorI)obj;
            }
            catch(System.InvalidCastException)
            {
                try
                {
                    IceInternal.Connector e = (IceInternal.Connector)obj;
                    return type() < e.type() ? -1 : 1;
                }
                catch(System.InvalidCastException)
                {
                    Debug.Assert(false);
                }
            }

            if(this == p)
            {
                return 0;
            }

            if(timeout_ < p.timeout_)
            {
                return -1;
            }
            else if(p.timeout_ < timeout_)
            {
                return 1;
            }

            if(!connectionId_.Equals(p.connectionId_))
            {
                return connectionId_.CompareTo(p.connectionId_);
            }

            return IceInternal.Network.compareAddress(addr_, p.addr_);
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
                    info.stream.EndAuthenticateAsClient(ar);
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

        internal bool validate(object sender, X509Certificate certificate, X509Chain chain,
                               SslPolicyErrors sslPolicyErrors)
        {
            string message = "";
            int errors = (int)sslPolicyErrors;
            if((errors & (int)SslPolicyErrors.RemoteCertificateNameMismatch) > 0)
            {
                if(!instance_.checkCertName())
                {
                    errors ^= (int)SslPolicyErrors.RemoteCertificateNameMismatch;
                    message = message + "\nremote certificate name mismatch (ignored)";
                }
                else
                {
                    if(instance_.securityTraceLevel() >= 1)
                    {
                        logger_.trace(instance_.securityTraceCategory(),
                                      "SSL certificate validation failed - remote certificate name mismatch");
                    }
                    return false;
                }
            }

            //
            // The RemoteCertificateNotAvailable case does not appear to be possible
            // for an outgoing connection. Since .NET requires an authenticated
            // connection, the remote peer closes the socket if it does not have a
            // certificate to provide.
            //

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

        private Instance instance_;
        private string host_;
        private Ice.Logger logger_;
        private IPEndPoint addr_;
        private int timeout_;
        private string connectionId_;
        private int hashCode_;
    }

    internal class ConnectorValidationCallback
    {
        internal ConnectorValidationCallback(ConnectorI connector)
        {
            connector_ = connector;
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
            return connector_.validate(sender, certificate, chain, sslPolicyErrors);
        }

        private ConnectorI connector_;
        internal X509Certificate2[] certs;
    }
}
