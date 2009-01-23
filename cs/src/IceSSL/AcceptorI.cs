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
    using System.IO;
    using System.Net;
    using System.Net.Security;
    using System.Net.Sockets;
    using System.Security.Cryptography.X509Certificates;

    class AcceptorI : IceInternal.Acceptor
    {
        public void close()
        {
            Socket fd;
            lock(this)
            {
                fd = _fd;
                _fd = null;
            }
            if(fd != null)
            {
                if(_instance.networkTraceLevel() >= 1)
                {
                    string s = "stopping to accept ssl connections at " + ToString();
                    _logger.trace(_instance.networkTraceCategory(), s);
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

        public void listen()
        {
            IceInternal.Network.doListen(_fd, _backlog);

            if(_instance.networkTraceLevel() >= 1)
            {
                string s = "accepting ssl connections at " + ToString();
                _logger.trace(_instance.networkTraceCategory(), s);
            }
        }

        public IAsyncResult beginAccept(AsyncCallback callback, object state)
        {
            //
            // The plug-in may not be fully initialized.
            //
            if(!_instance.initialized())
            {
                Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
                ex.reason = "IceSSL: plug-in is not initialized";
                throw ex;
            }

            try
            {
                return _fd.BeginAccept(callback, state);
            }
            catch(SocketException ex)
            {
                throw new Ice.SocketException(ex);
            }
        }

        public IceInternal.Transceiver endAccept(IAsyncResult result)
        {
            Socket fd = null;
            try
            {
                fd = _fd.EndAccept(result);
            }
            catch(SocketException ex)
            {
                throw new Ice.SocketException(ex);
            }

            IceInternal.Network.setBlock(fd, true); // SSL requires a blocking socket.
            IceInternal.Network.setTcpBufSize(fd, _instance.communicator().getProperties(), _logger);

            if(_instance.networkTraceLevel() >= 1)
            {
                string s = "attempting to accept ssl connection\n" + IceInternal.Network.fdToString(fd);
                _logger.trace(_instance.networkTraceCategory(), s);
            }

            return new TransceiverI(_instance, fd, null, true, null, _adapterName);
        }

        public override string ToString()
        {
            return IceInternal.Network.addrToString(_addr);
        }

        internal int effectivePort()
        {
            return _addr.Port;
        }

        internal AcceptorI(Instance instance, string adapterName, string host, int port)
        {
            _instance = instance;
            _adapterName = adapterName;
            _logger = instance.communicator().getLogger();
            _backlog = instance.communicator().getProperties().getPropertyAsIntWithDefault("Ice.TCP.Backlog", 511);

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

            try
            {
                _addr = IceInternal.Network.getAddressForServer(host, port, _instance.protocolSupport());
                _fd = IceInternal.Network.createSocket(false, _addr.AddressFamily);
                IceInternal.Network.setBlock(_fd, false);
                IceInternal.Network.setTcpBufSize(_fd, _instance.communicator().getProperties(), _logger);
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
                    IceInternal.Network.setReuseAddress(_fd, true);
                }
                if(_instance.networkTraceLevel() >= 2)
                {
                    string s = "attempting to bind to ssl socket " + ToString();
                    _logger.trace(_instance.networkTraceCategory(), s);
                }
                _addr = IceInternal.Network.doBind(_fd, _addr);
            }
            catch(System.Exception)
            {
                _fd = null;
                throw;
            }
        }

        private Instance _instance;
        private string _adapterName;
        private Ice.Logger _logger;
        private Socket _fd;
        private int _backlog;
        private IPEndPoint _addr;
    }
}
