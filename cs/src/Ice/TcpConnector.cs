// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System;
    using System.Diagnostics;
    using System.Net;
    using System.Net.Sockets;

    sealed class TcpConnector : Connector
    {
        internal const short TYPE = 1;

        public Transceiver connect()
        {
            if(_traceLevels.network >= 2)
            {
                string s = "trying to establish tcp connection to " + ToString();
                _logger.trace(_traceLevels.networkCat, s);
            }

            try
            {
#if SILVERLIGHT
                Socket fd = Network.createSocket(false, _addr.AddressFamily == AddressFamily.InterNetworkV6 ?
                                                        AddressFamily.InterNetworkV6 : AddressFamily.InterNetwork);
#else
                Socket fd = Network.createSocket(false, _addr.AddressFamily);
                Network.setBlock(fd, false);
#endif
#if !COMPACT
                Network.setTcpBufSize(fd, _instance.initializationData().properties, _logger);
#endif

                //
                // Nonblocking connect is handled by the transceiver.
                //
                return new TcpTransceiver(_instance, fd, _addr, _proxy, false);
            }
            catch(Ice.LocalException ex)
            {
                if(_traceLevels.network >= 2)
                {
                    string s = "failed to establish tcp connection to " + ToString() + "\n" + ex;
                    _logger.trace(_traceLevels.networkCat, s);
                }
                throw;
            }
        }

        public short type()
        {
            return Ice.UDPEndpointType.value;
        }

        //
        // Only for use by TcpEndpoint
        //
        internal TcpConnector(Instance instance, EndPoint addr, NetworkProxy proxy, int timeout, string connectionId)
        {
            _instance = instance;
            _traceLevels = instance.traceLevels();
            _logger = instance.initializationData().logger;
            _addr = addr;
            _proxy = proxy;
            _timeout = timeout;
            _connectionId = connectionId;

            _hashCode = 5381;
            IceInternal.HashUtil.hashAdd(ref _hashCode, _addr);
            IceInternal.HashUtil.hashAdd(ref _hashCode, _timeout);
            IceInternal.HashUtil.hashAdd(ref _hashCode, _connectionId);
        }

        public override bool Equals(object obj)
        {
            if(!(obj is TcpConnector))
            {
                return false;
            }

            if(this == obj)
            {
                return true;
            }

            TcpConnector p = (TcpConnector)obj;
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
            return Network.addrToString(_proxy == null ? _addr : _proxy.getAddress());
        }

        public override int GetHashCode()
        {
            return _hashCode;
        }

        private Instance _instance;
        private TraceLevels _traceLevels;
        private Ice.Logger _logger;
        private EndPoint _addr;
        private NetworkProxy _proxy;
        private int _timeout;
        private string _connectionId;
        private int _hashCode;
    }
}
