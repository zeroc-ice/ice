classdef IPConnectionInfo < Ice.ConnectionInfo
    % IPConnectionInfo   Summary of IPConnectionInfo
    %
    % Provides access to the connection details of an IP connection.
    %
    % IPConnectionInfo Properties:
    %   localAddress - The local address.
    %   localPort - The local port.
    %   remoteAddress - The remote address.
    %   remotePort - The remote port.

    %  Copyright (c) ZeroC, Inc.

    properties(SetAccess=immutable)
        % localAddress - The local address.
        localAddress char

        % localPort - The local port.
        localPort int32

        % remoteAddress - The remote address.
        remoteAddress char

        % remotePort - The remote port.
        remotePort int32
    end
    methods(Access=protected)
        function obj = IPConnectionInfo(connectionId, localAddress, localPort, remoteAddress, remotePort)
            assert(nargin == 5, 'Invalid number of arguments');
            obj@Ice.ConnectionInfo([], connectionId);
            obj.localAddress = localAddress;
            obj.localPort = localPort;
            obj.remoteAddress = remoteAddress;
            obj.remotePort = remotePort;
        end
    end
end
