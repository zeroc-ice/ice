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

    % Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

    methods
        function obj = IPConnectionInfo(underlying, incoming, adapterName, connectionId, localAddress, localPort, ...
                                        remoteAddress, remotePort)
            if nargin == 0
                underlying = [];
                incoming = false;
                adapterName = '';
                connectionId = '';
                localAddress = '';
                localPort = 0;
                remoteAddress = '';
                remotePort = 0;
            end
            obj = obj@Ice.ConnectionInfo(underlying, incoming, adapterName, connectionId);
            obj.localAddress = localAddress;
            obj.localPort = localPort;
            obj.remoteAddress = remoteAddress;
            obj.remotePort = remotePort;
        end
    end
    properties(SetAccess=private)
        % localAddress - The local address.
        localAddress char

        % localPort - The local port.
        localPort int32

        % remoteAddress - The remote address.
        remoteAddress char

        % remotePort - The remote port.
        remotePort int32
    end
end
