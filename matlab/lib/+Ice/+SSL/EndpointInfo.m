classdef (Sealed) EndpointInfo < Ice.EndpointInfo
    % TCPEndpointInfo   Summary of TCPEndpointInfo
    %
    % Provides access to a TCP endpoint information.

    % Copyright (c) ZeroC, Inc.

    methods
        function obj = EndpointInfo(underlying)
            assert(nargin == 1, 'Invalid number of arguments');
            obj@Ice.EndpointInfo(underlying);
        end
    end
end
