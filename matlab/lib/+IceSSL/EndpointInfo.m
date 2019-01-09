classdef EndpointInfo < Ice.EndpointInfo
    % TCPEndpointInfo   Summary of TCPEndpointInfo
    %
    % Provides access to a TCP endpoint information.

    % Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

    methods
        function obj = EndpointInfo(type, secure, underlying, timeout, compress)
            if nargin == 0
                underlying = [];
                timeout = 0;
                compress = false;
            end
            obj = obj@Ice.EndpointInfo(type, false, secure, underlying, timeout, compress);
        end
    end
end
