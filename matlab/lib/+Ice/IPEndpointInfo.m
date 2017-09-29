%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef IPEndpointInfo < Ice.EndpointInfo
    methods
        function obj = IPEndpointInfo(type, datagram, secure, underlying, timeout, compress, host, port, sourceAddress)
            if nargin == 3
                underlying = [];
                timeout = 0;
                compress = false;
                host = '';
                port = 0;
                sourceAddress = '';
            end
            obj = obj@Ice.EndpointInfo(type, datagram, secure, underlying, timeout, compress);
            obj.host = host;
            obj.port = port;
            obj.sourceAddress = sourceAddress;
        end
    end
    properties(SetAccess=private)
        host char
        port int32
        sourceAddress char
    end
end
