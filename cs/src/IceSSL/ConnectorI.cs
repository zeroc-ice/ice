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
    using System.Diagnostics;
    using System.IO;
    using System.Net;
    using System.Net.Sockets;

    sealed class ConnectorI : IceInternal.Connector
    {
        internal const short TYPE = 2;

        public IceInternal.Transceiver connect()
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

            if(_instance.networkTraceLevel() >= 2)
            {
                string s = "trying to establish ssl connection to " + ToString();
                _logger.trace(_instance.networkTraceCategory(), s);
            }

            try
            {
                Socket fd = IceInternal.Network.createSocket(false, _addr.AddressFamily);
                IceInternal.Network.setBlock(fd, true); // SSL requires a blocking socket.

                //
                // Windows XP has an IPv6 bug that makes a socket appear to be unconnected if you
                // set the socket's receive buffer size, and this in turn causes .NET to raise an
                // exception that would prevent us from using SSL.
                //
                if(_addr.AddressFamily != AddressFamily.InterNetworkV6 || !IceInternal.AssemblyUtil.xp_)
                {
                    IceInternal.Network.setTcpBufSize(fd, _instance.communicator().getProperties(), _logger);
                }

                //
                // Nonblocking connect is handled by the transceiver.
                //
                return new TransceiverI(_instance, fd, _host, false, false, null, _addr, _proxy);
            }
            catch(Ice.LocalException ex)
            {
                if(_instance.networkTraceLevel() >= 2)
                {
                    string s = "failed to establish ssl connection to " + ToString() + "\n" + ex;
                    _logger.trace(_instance.networkTraceCategory(), s);
                }
                throw;
            }
        }

        public short type()
        {
            return TYPE;
        }

        //
        // Only for use by EndpointI.
        //
        internal ConnectorI(Instance instance, string host, EndPoint addr, IceInternal.NetworkProxy proxy, int timeout, 
                            string conId)
        {
            _instance = instance;
            _host = host;
            _logger = instance.communicator().getLogger();
            _addr = (IPEndPoint)addr;
            _proxy = proxy; 
            _timeout = timeout;
            _connectionId = conId;

            _hashCode = 5381;
            IceInternal.HashUtil.hashAdd(ref _hashCode, _addr);
            IceInternal.HashUtil.hashAdd(ref _hashCode, _timeout);
            IceInternal.HashUtil.hashAdd(ref _hashCode, _connectionId);
        }

        public override bool Equals(object obj)
        {
            if(!(obj is ConnectorI))
            {
                return false;
            }

            if(this == obj)
            {
                return true;
            }

            ConnectorI p = (ConnectorI)obj;
            if(_timeout != p._timeout)
            {
                return false;
            }

            if(!_connectionId.Equals(p._connectionId))
            {
                return false;
            }

            return _addr.Equals(p._addr);
        }

        public override string ToString()
        {
            return IceInternal.Network.addrToString(_proxy == null ? _addr : _proxy.getAddress());
        }

        public override int GetHashCode()
        {
            return _hashCode;
        }

        private Instance _instance;
        private Ice.Logger _logger;
        private string _host;
        private IPEndPoint _addr;
        private IceInternal.NetworkProxy _proxy;
        private int _timeout;
        private string _connectionId;
        private int _hashCode;
    }
}
