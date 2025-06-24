classdef IPEndpointInfo < Ice.EndpointInfo
    %IPENDPOINTINFO Provides access to the address details of a IP endpoint.
    %
    %   IPEndpointInfo Properties:
    %     host - The host or address configured with the endpoint.
    %     port - The port number.
    %     sourceAddress - The source IP address.

    % Copyright (c) ZeroC, Inc.

    properties(SetAccess=immutable)
        %HOST The host or address configured with the endpoint.
        %   character vector
        host (1, :) char

        %PORT The port number.
        %   int32 scalar
        port (1, 1) int32

        %SOURCEADDRESS The source IP address.
        %   character vector
        sourceAddress (1, :) char
    end
    methods(Hidden, Access=protected)
        function obj = IPEndpointInfo(timeout, compress, host, port, sourceAddress)
            assert(nargin == 5, 'Invalid number of arguments');
            obj@Ice.EndpointInfo(Ice.EndpointInfo.empty, timeout, compress);
            obj.host = host;
            obj.port = port;
            obj.sourceAddress = sourceAddress;
        end
    end
end
