classdef (Sealed) OpaqueEndpointInfo < Ice.EndpointInfo
    %OPAQUEENDPOINTINFO Provides access to the details of an opaque endpoint.
    %
    %   OpaqueEndpointInfo Properties:
    %     rawEncoding - The encoding version of the opaque endpoint (to decode or encode the rawBytes).
    %     rawBytes - The raw encoding of the opaque endpoint.
    %
    %   OpaqueEndpointInfo Methods:
    %     type - Returns the type of the endpoint.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden)
        function obj = OpaqueEndpointInfo(type, rawEncoding, rawBytes)
            obj@Ice.EndpointInfo(Ice.EndpointInfo.empty, false);
            obj.type_ = type;
            obj.rawEncoding = rawEncoding;
            obj.rawBytes = rawBytes;
        end
    end
    methods
        function r = type(obj)
            r = obj.type_;
        end
    end
    properties (SetAccess = immutable)
        %RAWENCODING The encoding version of the opaque endpoint (to decode or encode the rawBytes).
        %   Ice.EncodingVersion scalar
        rawEncoding (1, 1) Ice.EncodingVersion

        %RAWBYTES The raw encoding of the opaque endpoint.
        %   uint8 vector
        rawBytes (1, :) uint8
    end
    properties (GetAccess = private, SetAccess = immutable)
        type_ (1, 1) int16
    end
end
