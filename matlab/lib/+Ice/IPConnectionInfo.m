%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef IPConnectionInfo < Ice.ConnectionInfo
    methods
        function obj = IPConnectionInfo(underlying, incoming, adapterName, connectionId, localAddress, localPort, ...
                                        remoteAddress, remotePort)
            if nargin == 0
                underlying = [];
                incoming = false;
                adapterName = '';
                connectionId = '';
                localAddress = '';
                localPort = 0;
                remoteAddress = '';
                remotePort = 0;
            end
            obj = obj@Ice.ConnectionInfo(underlying, incoming, adapterName, connectionId);
            obj.localAddress = localAddress;
            obj.localPort = localPort;
            obj.remoteAddress = remoteAddress;
            obj.remotePort = remotePort;
        end
    end
    properties(SetAccess=private)
        localAddress char
        localPort int32
        remoteAddress char
        remotePort int32
    end
end
