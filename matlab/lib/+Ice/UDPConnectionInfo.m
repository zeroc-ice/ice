classdef (Sealed) UDPConnectionInfo < Ice.IPConnectionInfo
    % UDPConnectionInfo   Summary of UDPConnectionInfo
    %
    % Provides access to the connection details of a UDP connection.
    %
    % UDPConnectionInfo Properties:
    %   mcastAddress - The multicast address.
    %   mcastPort - The multicast port.
    %   rcvSize - The connection buffer receive size.
    %   sndSize - The connection buffer send size.

    % Copyright (c) ZeroC, Inc.

    methods
        function obj = UDPConnectionInfo(connectionId, localAddress, localPort, remoteAddress, remotePort, ...
                                         mcastAddress, mcastPort, rcvSize, sndSize)
            assert(nargin == 9, 'Invalid number of arguments');
            obj@Ice.IPConnectionInfo(connectionId, localAddress, localPort, remoteAddress, remotePort);
            obj.mcastAddress = mcastAddress;
            obj.mcastPort = mcastPort;
            obj.rcvSize = rcvSize;
            obj.sndSize = sndSize;
        end
    end
    properties(SetAccess=immutable)
        % mcastAddress - The multicast address.
        mcastAddress (1, :) char

        % mcastPort - The multicast port.
        mcastPort (1, 1) int32

        % rcvSize - The connection buffer receive size.
        rcvSize (1, 1) int32

        % sndSize - The connection buffer send size.
        sndSize (1, 1) int32
    end
end
