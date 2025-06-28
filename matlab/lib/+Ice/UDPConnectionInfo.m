classdef (Sealed) UDPConnectionInfo < Ice.IPConnectionInfo
    %UDPCONNECTIONINFO Provides access to the connection details of a UDP connection.
    %
    %   UDPConnectionInfo Properties:
    %     mcastAddress - The multicast address.
    %     mcastPort - The multicast port.
    %     rcvSize - The connection buffer receive size.
    %     sndSize - The connection buffer send size.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden)
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
    properties (SetAccess = immutable)
        %MCASTADDRESS The multicast address.
        %   character vector
        mcastAddress (1, :) char

        %MCASTPORT The multicast port.
        %   int32 scalar
        mcastPort (1, 1) int32

        %RCVSIZE The connection buffer receive size.
        %   int32 scalar
        rcvSize (1, 1) int32

        %SNDSIZE The connection buffer send size.
        %   int32 scalar
        sndSize (1, 1) int32
    end
end
