classdef TCPConnectionInfo < Ice.IPConnectionInfo
    % TCPConnectionInfo   Summary of TCPConnectionInfo
    %
    % Provides access to the connection details of a TCP connection.
    %
    % TCPConnectionInfo Properties:
    %   rcvSize - The connection buffer receive size.
    %   sndSize - The connection buffer send size.

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

    methods
        function obj = TCPConnectionInfo(underlying, incoming, adapterName, connectionId, localAddress, localPort, ...
                                         remoteAddress, remotePort, rcvSize, sndSize)
            if nargin == 0
                underlying = [];
                incoming = false;
                adapterName = '';
                connectionId = '';
                localAddress = '';
                localPort = 0;
                remoteAddress = '';
                remotePort = 0;
                rcvSize = 0;
                sndSize = 0;
            end
            obj = obj@Ice.IPConnectionInfo(underlying, incoming, adapterName, connectionId, localAddress, ...
                                           localPort, remoteAddress, remotePort);
            obj.rcvSize = rcvSize;
            obj.sndSize = sndSize;
        end
    end
    properties(SetAccess=private)
        % rcvSize - The connection buffer receive size.
        rcvSize int32

        % The connection buffer send size.
        sndSize int32
    end
end
