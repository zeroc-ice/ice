classdef TCPEndpointInfo < Ice.IPEndpointInfo
    % TCPEndpointInfo   Summary of TCPEndpointInfo
    %
    % Provides access to a TCP endpoint information.

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

    methods
        function obj = TCPEndpointInfo(underlying, timeout, compress, host, port, sourceAddress)
            if nargin == 0
                underlying = [];
                timeout = 0;
                compress = false;
                host = '';
                port = 0;
                sourceAddress = '';
            end
            obj = obj@Ice.IPEndpointInfo(Ice.TCPEndpointType.value, false, false, underlying, timeout, ...
                                         compress, host, port, sourceAddress);
        end
    end
end
