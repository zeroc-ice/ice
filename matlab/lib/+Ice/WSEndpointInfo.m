classdef (Sealed) WSEndpointInfo < Ice.EndpointInfo
    %WSENDPOINTINFO Provides access to WebSocket endpoint information.
    %
    %   WSEndpointInfo Properties:
    %     resource - The URI configured with the endpoint.

    % Copyright (c) ZeroC, Inc.

    methods(Hidden)
        function obj = WSEndpointInfo(underlying, resource)
            assert(nargin == 2, 'Invalid number of arguments');
            obj@Ice.EndpointInfo(underlying);
            obj.resource = resource;
        end
    end
    properties(SetAccess=immutable)
        %RESOURCE The URI configured with the endpoint.
        %   character vector
        resource (1, :) char
    end
end
