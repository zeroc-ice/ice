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

    sealed class UdpConnector : Connector
    {
        public Transceiver connect()
        {
            return new UdpTransceiver(_instance, _addr, _mcastInterface, _mcastTtl);
        }

        public short type()
        {
            return _instance.type();
        }

        //
        // Only for use by UdpEndpointI
        //
        internal UdpConnector(ProtocolInstance instance, EndPoint addr, string mcastInterface, int mcastTtl,
                              string connectionId)
        {
            _instance = instance;
            _addr = addr;
            _mcastInterface = mcastInterface;
            _mcastTtl = mcastTtl;
            _connectionId = connectionId;

            _hashCode = 5381;
            IceInternal.HashUtil.hashAdd(ref _hashCode, _addr);
            IceInternal.HashUtil.hashAdd(ref _hashCode, _mcastInterface);
            IceInternal.HashUtil.hashAdd(ref _hashCode, _mcastTtl);
            IceInternal.HashUtil.hashAdd(ref _hashCode, _connectionId);
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

        private ProtocolInstance _instance;
        private EndPoint _addr;
        private string _mcastInterface;
        private int _mcastTtl;
        private string _connectionId;
        private int _hashCode;
    }
}
