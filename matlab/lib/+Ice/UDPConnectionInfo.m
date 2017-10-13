classdef UDPConnectionInfo < Ice.IPConnectionInfo
    % UDPConnectionInfo   Summary of UDPConnectionInfo
    %
    % Provides access to the connection details of a UDP connection.
    %
    % UDPConnectionInfo Properties:
    %   mcastAddress - The multicast address.
    %   mcastPort - The multicast port.
    %   rcvSize - The connection buffer receive size.
    %   sndSize - The connection buffer send size.

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

    methods
        function obj = UDPConnectionInfo(underlying, incoming, adapterName, connectionId, localAddress, localPort, ...
                                         remoteAddress, remotePort, mcastAddress, mcastPort, rcvSize, sndSize)
            if nargin == 0
                underlying = [];
                incoming = false;
                adapterName = '';
                connectionId = '';
                localAddress = '';
                localPort = 0;
                remoteAddress = '';
                remotePort = 0;
                mcastAddress = '';
                mcastPort = 0;
                rcvSize = 0;
                sndSize = 0;
            end
            obj = obj@Ice.IPConnectionInfo(underlying, incoming, adapterName, connectionId, localAddress, ...
                                           localPort, remoteAddress, remotePort);
            obj.mcastAddress = mcastAddress;
            obj.mcastPort = mcastPort;
            obj.rcvSize = rcvSize;
            obj.sndSize = sndSize;
        end
    end
    properties(SetAccess=private)
        % mcastAddress - The multicast address.
        mcastAddress char

        % mcastPort - The multicast port.
        mcastPort int32

        % rcvSize - The connection buffer receive size.
        rcvSize int32

        % sndSize - The connection buffer send size.
        sndSize int32
    end
end
