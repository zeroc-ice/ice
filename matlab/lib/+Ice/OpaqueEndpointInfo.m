classdef (Sealed) OpaqueEndpointInfo < Ice.EndpointInfo
    % OpaqueEndpointInfo   Summary of OpaqueEndpointInfo
    %
    % Provides access to the details of an opaque endpoint.
    %
    % OpaqueEndpointInfo Properties:
    %   rawEncoding - The encoding version of the opaque endpoint (to decode or encode the rawBytes).
    %   rawBytes - The raw encoding of the opaque endpoint.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    methods
        function obj = OpaqueEndpointInfo(type, rawEncoding, rawBytes)
            obj@Ice.EndpointInfo([], -1, false);
            obj.type_ = type;
            obj.rawEncoding = rawEncoding;
            obj.rawBytes = rawBytes;
        end

        function r = type(obj)
            % type   Returns the type of the endpoint.
            %
            % Returns (int16) - The endpoint type.

            r = obj.type_;
        end
    end
    properties(SetAccess=immutable)
        % rawEncoding - The encoding version of the opaque endpoint (to decode
        %   or encode the rawBytes).
        rawEncoding

        % rawBytes - The raw encoding of the opaque endpoint.
        rawBytes uint8
    end
    properties(GetAccess=private, SetAccess=immutable)
        type_ int16
    end
end
