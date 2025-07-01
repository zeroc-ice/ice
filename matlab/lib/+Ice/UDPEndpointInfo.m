classdef (Sealed) UDPEndpointInfo < Ice.IPEndpointInfo
    %UDPENDPOINTINFO Provides access to UDP endpoint information.
    %
    %   UDPEndpointInfo Properties:
    %     mcastInterface - The multicast interface.
    %     mcastTtl - The multicast time-to-live (or hops).
    %
    %   UDPEndpointInfo Methods:
    %     type - Returns the type of the endpoint.
    %     datagram - Returns true if this endpoint is a datagram endpoint.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden)
        function obj = UDPEndpointInfo(compress, host, port, sourceAddress, mcastInterface, mcastTtl)
            assert(nargin == 6, 'Invalid number of arguments');
            obj@Ice.IPEndpointInfo(compress, host, port, sourceAddress);
            obj.mcastInterface = mcastInterface;
            obj.mcastTtl = mcastTtl;
        end
    end
    methods
        function r = type(~)
            r = Ice.UDPEndpointType.value;
        end

        function r = datagram(~)
            r = true;
        end
    end
    properties (SetAccess = immutable)
        %MCASTINTERFACE The multicast interface.
        %   character vector
        mcastInterface (1, :) char

        %MCASTTTL The multicast time-to-live (or hops).
        %   int32 scalar
        mcastTtl (1, 1) int32
    end
end
