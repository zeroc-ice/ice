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

    sealed class UdpConnector : Connector
    {
        public Transceiver connect()
        {
            return new UdpTransceiver(instance_, _endpointInfo, _addr);
        }

        public short type()
        {
            return UdpEndpointI.TYPE;
        }

        //
        // Only for use by TcpEndpoint
        //
        internal UdpConnector(Instance instance, Ice.UdpEndpointInfo endpointInfo, IPEndPoint addr, string connectionId)
        {
            instance_ = instance;
            _endpointInfo = endpointInfo;
            _addr = addr;
            _connectionId = connectionId;

            _hashCode = _addr.GetHashCode();
            _hashCode = 5 * _hashCode + _endpointInfo.mcastInterface.GetHashCode();
            _hashCode = 5 * _hashCode + _endpointInfo.mcastTtl.GetHashCode();
            _hashCode = 5 * _hashCode + _connectionId.GetHashCode();
        }

        public override bool Equals(object obj)
        {
            UdpConnector p = null;

            try
            {
                p = (UdpConnector)obj;
            }
            catch(InvalidCastException)
            {
                return false;
            }

            if(this == p)
            {
                return true;
            }

            if(!_connectionId.Equals(p._connectionId))
            {
                return false;
            }

            if(_endpointInfo.protocolMajor != p._endpointInfo.protocolMajor)
            {
                return false;
            }

            if(_endpointInfo.protocolMinor != p._endpointInfo.protocolMinor)
            {
                return false;
            }

            if(_endpointInfo.encodingMajor != p._endpointInfo.encodingMajor)
            {
                return false;
            }

            if(_endpointInfo.encodingMinor != p._endpointInfo.encodingMinor)
            {
                return false;
            }

            if(!_endpointInfo.mcastInterface.Equals(p._endpointInfo.mcastInterface))
            {
                return false;
            }

            if(_endpointInfo.mcastTtl != p._endpointInfo.mcastTtl)
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

        private Instance instance_;
        private Ice.UdpEndpointInfo _endpointInfo;
        private IPEndPoint _addr;
        private string _connectionId;
        private int _hashCode;
    }
}
