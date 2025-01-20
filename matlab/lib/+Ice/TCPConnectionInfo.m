classdef (Sealed) TCPConnectionInfo < Ice.IPConnectionInfo
    % TCPConnectionInfo   Summary of TCPConnectionInfo
    %
    % Provides access to the connection details of a TCP connection.
    %
    % TCPConnectionInfo Properties:
    %   rcvSize - The connection buffer receive size.
    %   sndSize - The connection buffer send size.

    %  Copyright (c) ZeroC, Inc.

    methods
        function obj = TCPConnectionInfo(connectionId, localAddress, localPort, remoteAddress, remotePort, ...
                                         rcvSize, sndSize)
            assert(nargin == 7, 'Invalid number of arguments');
            obj@Ice.IPConnectionInfo(connectionId, localAddress, localPort, remoteAddress, remotePort);
            obj.rcvSize = rcvSize;
            obj.sndSize = sndSize;
        end
    end
    properties(SetAccess=immutable)
        % rcvSize - The connection buffer receive size.
        rcvSize int32

        % The connection buffer send size.
        sndSize int32
    end
end
