// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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

    sealed class ConnectorI : IceInternal.Connector, IComparable
    {
        internal const short TYPE = 2;

        public IceInternal.Transceiver connect()
        {
            //
            // The plugin may not be fully initialized.
            //
            if(!_instance.initialized())
            {
                Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
                ex.reason = "IceSSL: plugin is not initialized";
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
                return new TransceiverI(_instance, fd, _addr, false, _host, null);
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

            if(_timeout < p._timeout)
            {
                return -1;
            }
            else if(p._timeout < _timeout)
            {
                return 1;
            }

            if(!_connectionId.Equals(p._connectionId))
            {
                return _connectionId.CompareTo(p._connectionId);
            }

            return IceInternal.Network.compareAddress(_addr, p._addr);
        }

        //
        // Only for use by EndpointI.
        //
        internal ConnectorI(Instance instance, IPEndPoint addr, int timeout, string connectionId)
        {
            _instance = instance;
            _host = addr.Address.ToString();
            _logger = instance.communicator().getLogger();
            _addr = addr;
            _timeout = timeout;
            _connectionId = connectionId;

            _hashCode = _addr.GetHashCode();
            _hashCode = 5 * _hashCode + _timeout;
            _hashCode = 5 * _hashCode + _connectionId.GetHashCode();
        }

        public override bool Equals(object obj)
        {
            return CompareTo(obj) == 0;
        }

        public override string ToString()
        {
            return IceInternal.Network.addrToString(_addr);
        }

        public override int GetHashCode()
        {
            return _hashCode;
        }

        private Instance _instance;
        private string _host;
        private Ice.Logger _logger;
        private IPEndPoint _addr;
        private int _timeout;
        private string _connectionId;
        private int _hashCode;
    }
}
