classdef (Sealed) TCPEndpointInfo < Ice.IPEndpointInfo
    %TCPENDPOINTINFO Provides access to a TCP endpoint information.
    %
    %   TCPEndpointInfo Methods:
    %     type - Returns the type of the endpoint.
    %     secure - Returns true if this endpoint is a secure endpoint.

    % Copyright (c) ZeroC, Inc.

    methods(Hidden)
        function obj = TCPEndpointInfo(compress, host, port, sourceAddress, type, secure)
            assert(nargin == 6, 'Invalid number of arguments');
            obj@Ice.IPEndpointInfo(compress, host, port, sourceAddress);
            obj.type_ = type;
            obj.secure_ = secure;
        end
    end
    methods
        function r = type(obj)
            r = obj.type_;
        end

        function r = secure(obj)
            r = obj.secure_;
        end
    end
    properties(GetAccess=private, SetAccess=immutable)
        type_ (1, 1) int16
        secure_ (1, 1) logical
    end
end
