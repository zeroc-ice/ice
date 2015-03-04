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
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Net;
    using System.Net.Security;
    using System.Net.Sockets;
    using System.Security.Cryptography.X509Certificates;
    using System.Text;

    class AcceptorI : IceInternal.Acceptor
    {
        public void close()
        {
            if(_instance.networkTraceLevel() >= 1)
            {
                string s = "stopping to accept ssl connections at " + ToString();
                _logger.trace(_instance.networkTraceCategory(), s);
            }
            
            Debug.Assert(_acceptFd == null);
            
            try
            {
                _fd.Close();
                _fd = null;
            }
            catch(System.Exception)
            {
                // Ignore.
            }
        }

        public void listen()
        {
            IceInternal.Network.doListen(_fd, _backlog);

            if(_instance.networkTraceLevel() >= 1)
            {
                StringBuilder s = new StringBuilder("listening for ssl connections at ");
                s.Append(ToString());

                List<string> interfaces = 
                    IceInternal.Network.getHostsForEndpointExpand(_addr.Address.ToString(), 
                                                                  _instance.protocolSupport(), true);
                if(interfaces.Count != 0)
                {
                    s.Append("\nlocal interfaces: ");
                    s.Append(String.Join(", ", interfaces.ToArray()));
                }
                _logger.trace(_instance.networkTraceCategory(), s.ToString());
            }
        }

        public bool startAccept(IceInternal.AsyncCallback callback, object state)
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
                _result = _fd.BeginAccept(delegate(IAsyncResult result)
                                          {
                                              if(!result.CompletedSynchronously)
                                              {
                                                  callback(result.AsyncState);
                                              }
                                          }, state);
                return _result.CompletedSynchronously;
            }
            catch(SocketException ex)
            {
                throw new Ice.SocketException(ex);
            }
        }

        public void finishAccept()
        {
            if(_fd != null)
            {
                Debug.Assert(_result != null);
                try
                {
                    _acceptFd = _fd.EndAccept(_result);
                    _result = null;
                }
                catch(SocketException ex)
                {
                    _acceptError = ex;
                }
            }
        }

        public IceInternal.Transceiver accept()
        {
            if(_acceptFd == null)
            {
                throw _acceptError;
            }

            IceInternal.Network.setBlock(_acceptFd, true); // SSL requires a blocking socket.
            IceInternal.Network.setTcpBufSize(_acceptFd, _instance.communicator().getProperties(), _logger);

            if(_instance.networkTraceLevel() >= 1)
            {
                string s = "attempting to accept ssl connection\n" + IceInternal.Network.fdToString(_acceptFd);
                _logger.trace(_instance.networkTraceCategory(), s);
            }

            Socket acceptFd = _acceptFd;
            _acceptFd = null;
            _acceptError = null;
            return new TransceiverI(_instance, acceptFd, "", true, true, _adapterName, null, null);
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
                int protocol = instance.protocolSupport();
                _addr = (IPEndPoint)IceInternal.Network.getAddressForServer(host, port, protocol, instance.preferIPv6());
                _fd = IceInternal.Network.createServerSocket(false, _addr.AddressFamily, protocol);
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
                    string s = "attempting to bind to ssl socket " + IceInternal.Network.addrToString(_addr);
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
        private Socket _acceptFd;
        private System.Exception _acceptError;
        private int _backlog;
        private IPEndPoint _addr;
        private IAsyncResult _result;
    }
}
