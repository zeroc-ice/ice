classdef IPConnectionInfo < Ice.ConnectionInfo
    %IPCONNECTIONINFO Provides access to the connection details of an IP connection.
    %
    %   IPConnectionInfo Properties:
    %     localAddress - The local address.
    %     localPort - The local port.
    %     remoteAddress - The remote address.
    %     remotePort - The remote port.

    % Copyright (c) ZeroC, Inc.

    properties (SetAccess = immutable)
        %LOCALADDRESS The local address.
        %  character vector
        localAddress (1, :) char

        %LOCALPORT The local port.
        %  int32 scalar
        localPort (1, 1) int32

        %REMOTEADDRESS The remote address.
        %  character vector
        remoteAddress (1, :) char

        %REMOTEPORT The remote port.
        %  int32 scalar
        remotePort (1, 1) int32
    end
    methods (Hidden, Access = protected)
        function obj = IPConnectionInfo(connectionId, localAddress, localPort, remoteAddress, remotePort)
            assert(nargin == 5, 'Invalid number of arguments');
            obj@Ice.ConnectionInfo(Ice.ConnectionInfo.empty, connectionId);
            obj.localAddress = localAddress;
            obj.localPort = localPort;
            obj.remoteAddress = remoteAddress;
            obj.remotePort = remotePort;
        end
    end
end
