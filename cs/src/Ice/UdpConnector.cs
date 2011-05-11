// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
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
        internal UdpConnector(Instance instance, IPEndPoint addr, string mcastInterface, int mcastTtl,
                              byte protocolMajor, byte protocolMinor, byte encodingMajor, byte encodingMinor,
                              string connectionId)
        {
            instance_ = instance;
            _addr = addr;
            _mcastInterface = mcastInterface;
            _mcastTtl = mcastTtl;
            _protocolMajor = protocolMajor;
            _protocolMinor = protocolMinor;
            _encodingMajor = encodingMajor;
            _encodingMinor = encodingMinor;
            _connectionId = connectionId;

            _hashCode = _addr.GetHashCode();
            _hashCode = 5 * _hashCode + _mcastInterface.GetHashCode();
            _hashCode = 5 * _hashCode + _mcastTtl.GetHashCode();
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

            if(_protocolMajor != p._protocolMajor)
            {
                return false;
            }

            if(_protocolMinor != p._protocolMinor)
            {
                return false;
            }

            if(_encodingMajor != p._encodingMajor)
            {
                return false;
            }

            if(_encodingMinor != p._encodingMinor)
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
        private IPEndPoint _addr;
        private string _mcastInterface;
        private int _mcastTtl;
        private byte _protocolMajor;
        private byte _protocolMinor;
        private byte _encodingMajor;
        private byte _encodingMinor;
        private string _connectionId;
        private int _hashCode;
    }
}
