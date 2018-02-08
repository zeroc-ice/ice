// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
            if(_acceptFd != null)
            {
                Network.closeSocketNoThrow(_acceptFd);
                _acceptFd = null;
            }
            if(_fd != null)
            {
                Network.closeSocketNoThrow(_fd);
                _fd = null;
            }
        }

        public virtual EndpointI listen()
        {
            try
            {
                _addr = Network.doBind(_fd, _addr);
                Network.doListen(_fd, _backlog);
            }
            catch(SystemException)
            {
                _fd = null;
                throw;
            }
            _endpoint = _endpoint.endpoint(this);
            return _endpoint;
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

            Socket acceptFd = _acceptFd;
            _acceptFd = null;
            _acceptError = null;
            return new TcpTransceiver(_instance, new StreamSocket(_instance, acceptFd));
        }

        public string protocol()
        {
            return _instance.protocol();
        }

        public override string ToString()
        {
            return Network.addrToString(_addr);
        }

        public string toDetailedString()
        {
            StringBuilder s = new StringBuilder("local address = ");
            s.Append(ToString());

            List<string> intfs =
                Network.getHostsForEndpointExpand(_addr.Address.ToString(), _instance.protocolSupport(), true);
            if(intfs.Count != 0)
            {
                s.Append("\nlocal interfaces = ");
                s.Append(String.Join(", ", intfs.ToArray()));
            }
            return s.ToString();
        }

        internal int effectivePort()
        {
            return _addr.Port;
        }

        internal TcpAcceptor(TcpEndpointI endpoint, ProtocolInstance instance, string host, int port)
        {
            _endpoint = endpoint;
            _instance = instance;
            _backlog = instance.properties().getPropertyAsIntWithDefault("Ice.TCP.Backlog", 511);

            try
            {
                int protocol = _instance.protocolSupport();
                _addr = (IPEndPoint)Network.getAddressForServer(host, port, protocol, _instance.preferIPv6());
                _fd = Network.createServerSocket(false, _addr.AddressFamily, protocol);
                Network.setBlock(_fd, false);
                Network.setTcpBufSize(_fd, _instance);
                if(!AssemblyUtil.isWindows)
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
                    Network.setReuseAddress(_fd, true);
                }
            }
            catch(Exception)
            {
                _fd = null;
                throw;
            }
        }

        private TcpEndpointI _endpoint;
        private ProtocolInstance _instance;
        private Socket _fd;
        private Socket _acceptFd;
        private Exception _acceptError;
        private int _backlog;
        private IPEndPoint _addr;
        private IAsyncResult _result;
    }
}
