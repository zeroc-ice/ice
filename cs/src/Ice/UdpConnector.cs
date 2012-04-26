// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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
            return new UdpTransceiver(instance_, _addr, _mcastInterface, _mcastTtl);
        }

        public short type()
        {
            return Ice.UDPEndpointType.value;
        }

        //
        // Only for use by TcpEndpoint
        //
        internal UdpConnector(Instance instance, EndPoint addr, string mcastInterface, int mcastTtl,
                              Ice.ProtocolVersion protocol, Ice.EncodingVersion encoding, string connectionId)
        {
            instance_ = instance;
#if SILVERLIGHT
            _addr = (DnsEndPoint)addr;
#else
            _addr = (IPEndPoint)addr;
#endif
            _mcastInterface = mcastInterface;
            _mcastTtl = mcastTtl;
            _protocol = protocol;
            _encoding = encoding;
            _connectionId = connectionId;

            _hashCode = _addr.GetHashCode();
            _hashCode = 5 * _hashCode + _mcastInterface.GetHashCode();
            _hashCode = 5 * _hashCode + _protocol.GetHashCode();
            _hashCode = 5 * _hashCode + _encoding.GetHashCode();
            _hashCode = 5 * _hashCode + _mcastTtl.GetHashCode();
            _hashCode = 5 * _hashCode + _connectionId.GetHashCode();
        }

        public override bool Equals(object obj)
        {
            if(!(obj is UdpConnector))
            {
                return false;
            }

            if(this == obj)
            {
                return true;
            }

            UdpConnector p = (UdpConnector)obj;
            if(!_connectionId.Equals(p._connectionId))
            {
                return false;
            }

            if(!_protocol.Equals(p._protocol))
            {
                return false;
            }

            if(!_encoding.Equals(p._encoding))
            {
                return false;
            }

            if(!_mcastInterface.Equals(p._mcastInterface))
            {
                return false;
            }

            if(_mcastTtl != p._mcastTtl)
            {
                return false;
            }

            return _addr.Equals(p._addr);
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
#if SILVERLIGHT
        private DnsEndPoint _addr;
#else
        private IPEndPoint _addr;
#endif
        private string _mcastInterface;
        private int _mcastTtl;
        private Ice.ProtocolVersion _protocol;
        private Ice.EncodingVersion _encoding;
        private string _connectionId;
        private int _hashCode;
    }
}
