classdef (Sealed) WSEndpointInfo < Ice.EndpointInfo
    % WSEndpointInfo   Summary of WSEndpointInfo
    %
    % Provides access to WebSocket endpoint information.
    %
    % WSEndpointInfo Properties:
    %   resource - The URI configured with the endpoint.

    %  Copyright (c) ZeroC, Inc.

    methods
        function obj = WSEndpointInfo(underlying, resource)
            assert(nargin == 2, 'Invalid number of arguments');
            obj@Ice.EndpointInfo(underlying);
            obj.resource = resource;
        end
    end
    properties(SetAccess=immutable)
        % resource - The URI configured with the endpoint.
        resource char
    end
end
