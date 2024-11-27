classdef IPEndpointInfo < Ice.EndpointInfo
    % IPEndpointInfo   Summary of IPEndpointInfo
    %
    % Provides access to the address details of a IP endpoint.
    %
    % IPEndpointInfo Properties:
    %   host - The host or address configured with the endpoint.
    %   port - The port number.
    %   sourceAddress - The source IP address.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    properties(SetAccess=immutable)
        % host - The host or address configured with the endpoint.
        host char

        % port - The port number.
        port int32

        % sourceAddress - The source IP address.
        sourceAddress char
    end

    methods(Access=protected)
        function obj = IPEndpointInfo(timeout, compress, host, port, sourceAddress)
            assert(nargin == 5, 'Invalid number of arguments');
            obj@Ice.EndpointInfo([], timeout, compress);
            obj.host = host;
            obj.port = port;
            obj.sourceAddress = sourceAddress;
        end
    end
end
