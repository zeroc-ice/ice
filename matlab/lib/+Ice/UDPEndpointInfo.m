classdef (Sealed) UDPEndpointInfo < Ice.IPEndpointInfo
    % UDPEndpointInfo   Summary of UDPEndpointInfo
    %
    % Provides access to UDP endpoint information.
    %
    % UDPEndpointInfo Properties:
    %   mcastInterface - The multicast interface.
    %   mcastTtl - The multicast time-to-live (or hops).

    % Copyright (c) ZeroC, Inc.

    methods
        function obj = UDPEndpointInfo(compress, host, port, sourceAddress, mcastInterface, mcastTtl)
            assert(nargin == 6, 'Invalid number of arguments');
            obj@Ice.IPEndpointInfo(-1, compress, host, port, sourceAddress);
            obj.mcastInterface = mcastInterface;
            obj.mcastTtl = mcastTtl;
        end

        function r = type(~)
            % type   Returns the type of the endpoint.
            %
            % Returns (int16) - The endpoint type.

            r = Ice.UDPEndpointType.value;
        end

        function r = datagram(~)
            % datagram   Returns true if this endpoint is a datagram endpoint.
            %
            % Returns (logical) - True for a datagram endpoint.

            r = true;
        end
    end
    properties(SetAccess=immutable)
        % mcastInterface - The multicast interface.
        mcastInterface char

        % mcastTtl - The multicast time-to-live (or hops).
        mcastTtl int32
    end
end
