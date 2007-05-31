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
    using System.Security.Cryptography.X509Certificates;

    class AcceptorI : IceInternal.Acceptor
    {
        public virtual Socket fd()
        {
            return fd_;
        }
        
        public virtual void close()
        {
            Socket fd;
            lock(this)
            {
                fd = fd_;
                fd_ = null;
            }
            if(fd != null)
            {
                if(instance_.networkTraceLevel() >= 1)
                {
                    string s = "stopping to accept ssl connections at " + ToString();
                    logger_.trace(instance_.networkTraceCategory(), s);
                }
            
                try
                {
                    fd.Close();
                }
                catch(System.Exception)
                {
                    // Ignore.
                }
            }
        }
        
        public virtual void listen()
        {
            IceInternal.Network.doListen(fd_, backlog_);
            
            if(instance_.networkTraceLevel() >= 1)
            {
                string s = "accepting ssl connections at " + ToString();
                logger_.trace(instance_.networkTraceCategory(), s);
            }
        }
        
        public virtual IceInternal.Transceiver accept(int timeout)
        {
            Debug.Assert(timeout == -1); // Always called with -1 for thread-per-connection.

            //
            // The plugin may not be fully initialized.
            //
            if(!instance_.initialized())
            {
                Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
                ex.reason = "IceSSL: plugin is not initialized";
                throw ex;
            }

            Socket fd = IceInternal.Network.doAccept(fd_, timeout);
            IceInternal.Network.setBlock(fd, true); // SSL requires a blocking socket.
            IceInternal.Network.setTcpBufSize(fd, instance_.communicator().getProperties(), logger_);

            if(instance_.networkTraceLevel() >= 1)
            {
                string s = "attempting to accept ssl connection\n" + IceInternal.Network.fdToString(fd);
                logger_.trace(instance_.networkTraceCategory(), s);
            }

            return new TransceiverI(instance_, fd, adapterName_);
        }

        public virtual void connectToSelf()
        {
            Socket fd = IceInternal.Network.createSocket(false);
            IceInternal.Network.setBlock(fd, false);
            IceInternal.Network.doConnect(fd, addr_, -1);
            IceInternal.Network.closeSocket(fd);
        }

        public override string ToString()
        {
            return IceInternal.Network.addrToString(addr_);
        }
        
        internal bool equivalent(string host, int port)
        {
            EndPoint addr = IceInternal.Network.getAddress(host, port);
            return addr.Equals(addr_);
        }

        internal virtual int effectivePort()
        {
            return addr_.Port;
        }

        internal
        AcceptorI(Instance instance, string adapterName, string host, int port)
        {
            instance_ = instance;
            adapterName_ = adapterName;
            logger_ = instance.communicator().getLogger();
            backlog_ = 0;

            //
            // .NET requires that a certificate be supplied.
            //
            X509Certificate2Collection certs = instance.certs();
            if(certs.Count == 0)
            {
                Ice.SecurityException ex = new Ice.SecurityException();
                ex.reason = "IceSSL: certificate required for server endpoint";
                throw ex;
            }

            if(backlog_ <= 0)
            {
                backlog_ = 5;
            }
            
            try
            {
                fd_ = IceInternal.Network.createSocket(false);
                IceInternal.Network.setBlock(fd_, false);
                IceInternal.Network.setTcpBufSize(fd_, instance_.communicator().getProperties(), logger_);
                addr_ = IceInternal.Network.getAddress(host, port);
                if(IceInternal.AssemblyUtil.platform_ != IceInternal.AssemblyUtil.Platform.Windows)
                {
                    //
                    // Enable SO_REUSEADDR on Unix platforms to allow
                    // re-using the socket even if it's in the TIME_WAIT
                    // state. On Windows, this doesn't appear to be
                    // necessary and enabling SO_REUSEADDR would actually
                    // not be a good thing since it allows a second
                    // process to bind to an address even it's already
                    // bound by another process.
                    //
                    // TODO: using SO_EXCLUSIVEADDRUSE on Windows would
                    // probably be better but it's only supported by recent
                    // Windows versions (XP SP2, Windows Server 2003).
                    //
                    IceInternal.Network.setReuseAddress(fd_, true);
                }
                if(instance_.networkTraceLevel() >= 2)
                {
                    string s = "attempting to bind to ssl socket " + ToString();
                    logger_.trace(instance_.networkTraceCategory(), s);
                }
                addr_ = IceInternal.Network.doBind(fd_, addr_);
            }
            catch(System.Exception)
            {
                fd_ = null;
                throw;
            }
        }
        
#if DEBUG
        ~AcceptorI()
        {
            /*
            lock(this)
            {
                IceUtil.Assert.FinalizerAssert(fd_ == null);
            }
            */
        }
#endif

        private Instance instance_;
        private string adapterName_;
        private Ice.Logger logger_;
        private Socket fd_;
        private int backlog_;
        private IPEndPoint addr_;
    }
}
