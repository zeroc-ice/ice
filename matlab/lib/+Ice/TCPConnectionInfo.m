%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef TCPConnectionInfo < Ice.IPConnectionInfo
    methods
        function obj = TCPConnectionInfo(underlying, incoming, adapterName, connectionId, localAddress, localPort, ...
                                         remoteAddress, remotePort, rcvSize, sndSize)
            if nargin == 0
                underlying = [];
                incoming = false;
                adapterName = '';
                connectionId = '';
                localAddress = '';
                localPort = 0;
                remoteAddress = '';
                remotePort = 0;
                rcvSize = 0;
                sndSize = 0;
            end
            obj = obj@Ice.IPConnectionInfo(underlying, incoming, adapterName, connectionId, localAddress, ...
                                           localPort, remoteAddress, remotePort);
            obj.rcvSize = rcvSize;
            obj.sndSize = sndSize;
        end
    end
    properties(SetAccess=private)
        rcvSize int32
        sndSize int32
    end
end
