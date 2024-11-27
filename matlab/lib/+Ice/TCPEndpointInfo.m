classdef (Sealed) TCPEndpointInfo < Ice.IPEndpointInfo
    % TCPEndpointInfo   Summary of TCPEndpointInfo
    %
    % Provides access to a TCP endpoint information.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    methods
        function obj = TCPEndpointInfo(timeout, compress, host, port, sourceAddress, type, secure)
            assert(nargin == 7, 'Invalid number of arguments');
            obj@Ice.IPEndpointInfo(timeout, compress, host, port, sourceAddress);
            obj.type_ = type;
            obj.secure_ = secure;
        end

        function r = type(obj)
            % type   Returns the type of the endpoint.
            %
            % Returns (int16) - The endpoint type.

            r = obj.type_;
        end

        function r = secure(obj)
            % secure   Returns true if this endpoint is a secure endpoint.
            %
            % Returns (logical) - True for a secure endpoint.

            r = obj.secure_;
        end
    end

    properties(GetAccess=private, SetAccess=immutable)
        type_ int16
        secure_ logical
    end
end
