classdef (Sealed) TCPConnectionInfo < Ice.IPConnectionInfo
    %TCPCONNECTIONINFO Provides access to the connection details of a TCP connection.
    %
    %   TCPConnectionInfo Properties:
    %     rcvSize - The size of the receive buffer.
    %     sndSize - The size of the send buffer.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden)
        function obj = TCPConnectionInfo(connectionId, localAddress, localPort, remoteAddress, remotePort, ...
                                         rcvSize, sndSize)
            assert(nargin == 7, 'Invalid number of arguments');
            obj@Ice.IPConnectionInfo(connectionId, localAddress, localPort, remoteAddress, remotePort);
            obj.rcvSize = rcvSize;
            obj.sndSize = sndSize;
        end
    end
    properties (SetAccess = immutable)
        %RCVSIZE The size of the receive buffer.
        %   int32 scalar
        rcvSize (1, 1) int32

        %SNDSIZE The size of the send buffer.
        %   int32 scalar
        sndSize (1, 1) int32
    end
end
