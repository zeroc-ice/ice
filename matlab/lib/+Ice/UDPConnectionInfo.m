%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef UDPConnectionInfo < Ice.IPConnectionInfo
    methods
        function obj = UDPConnectionInfo(underlying, incoming, adapterName, connectionId, localAddress, localPort, ...
                                         remoteAddress, remotePort, mcastAddress, mcastPort, rcvSize, sndSize)
            if nargin == 0
                underlying = [];
                incoming = false;
                adapterName = '';
                connectionId = '';
                localAddress = '';
                localPort = 0;
                remoteAddress = '';
                remotePort = 0;
                mcastAddress = '';
                mcastPort = 0;
                rcvSize = 0;
                sndSize = 0;
            end
            obj = obj@Ice.IPConnectionInfo(underlying, incoming, adapterName, connectionId, localAddress, ...
                                           localPort, remoteAddress, remotePort);
            obj.mcastAddress = mcastAddress;
            obj.mcastPort = mcastPort;
            obj.rcvSize = rcvSize;
            obj.sndSize = sndSize;
        end
    end
    properties(SetAccess=private)
        mcastAddress char
        mcastPort int32
        rcvSize int32
        sndSize int32
    end
end
