classdef UDPEndpointInfo < Ice.IPEndpointInfo
    % UDPEndpointInfo   Summary of UDPEndpointInfo
    %
    % Provides access to UDP endpoint information.
    %
    % UDPEndpointInfo Properties:
    %   mcastInterface - The multicast interface.
    %   mcastTtl - The multicast time-to-live (or hops).

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

    methods
        function obj = UDPEndpointInfo(underlying, timeout, compress, host, port, sourceAddress, mcastInterface, ...
                                       mcastTtl)
            if nargin == 0
                underlying = [];
                timeout = 0;
                compress = false;
                host = '';
                port = 0;
                sourceAddress = '';
                mcastInterface = '';
                mcastTtl = 0;
            end
            obj = obj@Ice.IPEndpointInfo(Ice.UDPEndpointType.value, true, false, underlying, timeout, ...
                                         compress, host, port, sourceAddress);
            obj.mcastInterface = mcastInterface;
            obj.mcastTtl = mcastTtl;
        end
    end
    properties(SetAccess=private)
        % mcastInterface - The multicast interface.
        mcastInterface char

        % mcastTtl - The multicast time-to-live (or hops).
        mcastTtl int32
    end
end
