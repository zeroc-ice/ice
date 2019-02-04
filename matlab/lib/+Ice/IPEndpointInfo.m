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
        % host - The host or address configured with the endpoint.
        host char

        % port - The port number.
        port int32

        % sourceAddress - The source IP address.
        sourceAddress char
    end
end
