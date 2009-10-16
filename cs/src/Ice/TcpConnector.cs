// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
                Socket fd = Network.createSocket(false, _addr.AddressFamily);
                Network.setBlock(fd, false);
                Network.setTcpBufSize(fd, _instance.initializationData().properties, _logger);

                //
                // Nonblocking connect is handled by the transceiver.
                //
                return new TcpTransceiver(_instance, _endpointInfo, fd, _addr, false);
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
            return TcpEndpointI.TYPE;
        }

        //
        // Only for use by TcpEndpoint
        //
        internal TcpConnector(Instance instance, Ice.TcpEndpointInfo endpointInfo, IPEndPoint addr, string connectionId)
        {
            _instance = instance;
            _endpointInfo = endpointInfo;
            _traceLevels = instance.traceLevels();
            _logger = instance.initializationData().logger;
            _addr = addr;
            _connectionId = connectionId;

            _hashCode = _addr.GetHashCode();
            _hashCode = 5 * _hashCode + _endpointInfo.timeout;
            _hashCode = 5 * _hashCode + _connectionId.GetHashCode();
        }

        public override bool Equals(object obj)
        {
            TcpConnector p = null;

            try
            {
                p = (TcpConnector)obj;
            }
            catch(InvalidCastException)
            {
                return false;
            }

            if(this == p)
            {
                return true;
            }

            if(_endpointInfo.timeout != p._endpointInfo.timeout)
            {
                return false;
            }

            if(!_connectionId.Equals(p._connectionId))
            {
                return false;
            }

            return Network.compareAddress(_addr, p._addr) == 0;
        }

        public override string ToString()
        {
            return Network.addrToString(_addr);
        }

        public override int GetHashCode()
        {
            return _hashCode;
        }

        private Instance _instance;
        private Ice.TcpEndpointInfo _endpointInfo;
        private TraceLevels _traceLevels;
        private Ice.Logger _logger;
        private IPEndPoint _addr;
        private string _connectionId;
        private int _hashCode;
    }
}
