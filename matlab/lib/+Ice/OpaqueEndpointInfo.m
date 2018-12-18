classdef OpaqueEndpointInfo < Ice.EndpointInfo
    % OpaqueEndpointInfo   Summary of OpaqueEndpointInfo
    %
    % Provides access to the details of an opaque endpoint.
    %
    % OpaqueEndpointInfo Properties:
    %   rawEncoding - The encoding version of the opaque endpoint (to decode
    %     or encode the rawBytes).
    %   rawBytes - The raw encoding of the opaque endpoint.

    % Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

    methods
        function obj = OpaqueEndpointInfo(type, underlying, timeout, compress, rawEncoding, rawBytes)
            obj = obj@Ice.EndpointInfo(type, false, false, underlying, timeout, compress);
            obj.rawEncoding = rawEncoding;
            obj.rawBytes = rawBytes;
        end
    end
    properties(SetAccess=private)
        % rawEncoding - The encoding version of the opaque endpoint (to decode
        %   or encode the rawBytes).
        rawEncoding

        % rawBytes - The raw encoding of the opaque endpoint.
        rawBytes uint8
    end
end
