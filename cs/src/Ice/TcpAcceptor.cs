// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#if !SILVERLIGHT

namespace IceInternal
{

    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Net;
    using System.Net.Sockets;
    using System.Text;

    class TcpAcceptor : Acceptor
    {
        public virtual void close()
        {
            if(_instance.traceLevel() >= 1)
            {
                string s = "stopping to accept " + protocol() + " connections at " + ToString();
                _instance.logger().trace(_instance.traceCategory(), s);
            }

            Debug.Assert(_acceptFd == null);
            _fd.Close();
            _fd = null;
        }

        public virtual void listen()
        {
            Network.doListen(_fd, _backlog);

            if(_instance.traceLevel() >= 1)
            {
                StringBuilder s = new StringBuilder("listening for " + protocol() + " connections at ");
                s.Append(ToString());

                List<string> interfaces =
                    Network.getHostsForEndpointExpand(_addr.Address.ToString(), _instance.protocolSupport(), true);
                if(interfaces.Count != 0)
                {
                    s.Append("\nlocal interfaces: ");
                    s.Append(String.Join(", ", interfaces.ToArray()));
                }
                _instance.logger().trace(_instance.traceCategory(), s.ToString());
            }
        }

        public virtual bool startAccept(AsyncCallback callback, object state)
        {
            try
            {
                _result = _fd.BeginAccept(delegate(IAsyncResult result)
                                          {
                                              if(!result.CompletedSynchronously)
                                              {
                                                  callback(result.AsyncState);
                                              }
                                          }, state);
            }
            catch(SocketException ex)
            {
                throw new Ice.SocketException(ex);
            }
            return _result.CompletedSynchronously;
        }

        public virtual void finishAccept()
        {
            if(_fd != null)
            {
                _acceptFd = null;
                try
                {
                    _acceptFd = _fd.EndAccept(_result);
                }
                catch(SocketException ex)
                {
                    _acceptError = ex;
                }
            }
        }

        public virtual Transceiver accept()
        {
            if(_acceptFd == null)
            {
                throw _acceptError;
            }

            Network.setBlock(_acceptFd, false);
#  if !COMPACT
            Network.setTcpBufSize(_acceptFd, _instance.properties(), _instance.logger());
#  endif

            if(_instance.traceLevel() >= 1)
            {
                string s = "accepted " + protocol() + " connection\n" + Network.fdToString(_acceptFd);
                _instance.logger().trace(_instance.traceCategory(), s);
            }

            Socket acceptFd = _acceptFd;
            _acceptFd = null;
            _acceptError = null;
            return new TcpTransceiver(_instance, acceptFd, null, null, null, true);
        }

        public string protocol()
        {
            return _instance.protocol();
        }

        public override string ToString()
        {
            return Network.addrToString(_addr);
        }

        internal int effectivePort()
        {
            return _addr.Port;
        }

        internal TcpAcceptor(ProtocolInstance instance, string host, int port)
        {
            _instance = instance;
            _backlog = instance.properties().getPropertyAsIntWithDefault("Ice.TCP.Backlog", 511);

            try
            {
                int protocol = _instance.protocolSupport();
                _addr = (IPEndPoint)Network.getAddressForServer(host, port, protocol, _instance.preferIPv6());
                _fd = Network.createServerSocket(false, _addr.AddressFamily, protocol);
                Network.setBlock(_fd, false);
#  if !COMPACT
                Network.setTcpBufSize(_fd, _instance.properties(), _instance.logger());
#  endif
                if(AssemblyUtil.platform_ != AssemblyUtil.Platform.Windows)
                {
                    //
                    // Enable SO_REUSEADDR on Unix platforms to allow re-using the
                    // socket even if it's in the TIME_WAIT state. On Windows,
                    // this doesn't appear to be necessary and enabling
                    // SO_REUSEADDR would actually not be a good thing since it
                    // allows a second process to bind to an address even it's
                    // already bound by another process.
                    //
                    // TODO: using SO_EXCLUSIVEADDRUSE on Windows would probably
                    // be better but it's only supported by recent Windows
                    // versions (XP SP2, Windows Server 2003).
                    //
                    Network.setReuseAddress(_fd, true);
                }

                if(_instance.traceLevel() >= 2)
                {
                    string s = "attempting to bind to " + _instance.protocol() + " socket " +
                        Network.addrToString(_addr);
                    _instance.logger().trace(_instance.traceCategory(), s);
                }
                _addr = Network.doBind(_fd, _addr);
            }
            catch(System.Exception)
            {
                _fd = null;
                throw;
            }
        }

        private ProtocolInstance _instance;
        private Socket _fd;
        private Socket _acceptFd;
        private System.Exception _acceptError;
        private int _backlog;
        private IPEndPoint _addr;
        private IAsyncResult _result;
    }
}
#endif
