%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}
%
% Ice version 3.7.0
%

classdef Connection < IceInternal.WrapperObject
    methods
        function obj = Connection(impl, communicator)
            if ~isa(impl, 'lib.pointer')
                throw(MException('Ice:ArgumentException', 'invalid argument'));
            end
            obj = obj@IceInternal.WrapperObject(impl);
            obj.communicator = communicator;
        end
        function r = eq(obj, other)
            %
            % Override == operator.
            %
            if isempty(other) || ~isa(other, 'Ice.Connection')
                r = false;
            else
                %
                % Call into C++ to compare the two objects.
                %
                r = obj.iceCallWithResult('equals', other.impl_);
            end
        end
        function close(obj, mode)
            obj.iceCall('close', mode);
        end
        function f = closeAsync(obj)
            future = libpointer('voidPtr');
            obj.iceCall('closeAsync', future);
            assert(~isNull(future));
            f = Ice.Future(future, 'close', 0, 'Ice_SimpleFuture', @(fut) fut.iceCall('check'));
        end
        function r = createProxy(obj, id)
            proxy = libpointer('voidPtr');
            obj.iceCall('createProxy', id, proxy);
            r = Ice.ObjectPrx(obj.communicator, obj.communicator.getEncoding(), proxy);
        end
        function r = getEndpoint(obj)
            endpoint = libpointer('voidPtr');
            obj.iceCall('getEndpoint', endpoint);
            r = Ice.Endpoint(endpoint);
        end
        function flushBatchRequests(obj, compress)
            obj.iceCall('flushBatchRequests', compress);
        end
        function r = flushBatchRequestsAsync(obj)
            future = libpointer('voidPtr');
            obj.iceCall('flushBatchRequestsAsync', future);
            assert(~isNull(future));
            r = Ice.Future(future, 'flushBatchRequests', 0, 'Ice_SimpleFuture', @(fut) fut.iceCall('check'));
        end
        function heartbeat(obj)
            obj.iceCall('heartbeat');
        end
        function r = heartbeatAsync(obj)
            future = libpointer('voidPtr');
            obj.iceCall('heartbeatAsync', future);
            assert(~isNull(future));
            r = Ice.Future(future, 'heartbeat', 0, 'Ice_SimpleFuture', @(fut) fut.iceCall('check'));
        end
        function setACM(obj, timeout, close, heartbeat)
            if timeout == Ice.Unset
                timeout = [];
            end
            if close == Ice.Unset
                close = [];
            end
            if heartbeat == Ice.Unset
                heartbeat = [];
            end
            obj.iceCall('setACM', timeout, close, heartbeat);
        end
        function r = getACM(obj)
            r = obj.iceCallWithResult('getACM');
            if isempty(r.timeout)
                r.timeout = Ice.Unset;
            end
            if isempty(r.close)
                r.close = Ice.Unset;
            end
            if isempty(r.heartbeat)
                r.heartbeat = Ice.Unset;
            end
        end
        function r = type(obj)
            r = obj.iceCallWithResult('type');
        end
        function r = timeout(obj)
            r = obj.iceCallWithResult('timeout');
        end
        function r = toString(obj)
            r = obj.iceCallWithResult('toString');
        end
        function r = getInfo(obj)
            info = obj.iceCallWithResult('getInfo');
            r = obj.createConnectionInfo(info);
        end
        function setBufferSize(obj, rcvSize, sndSize)
            obj.iceCall('setBufferSize', rcvSize, sndSize);
        end
        function throwException(obj)
            obj.iceCall('throwException');
        end
    end

    methods(Access=private)
        function r = createConnectionInfo(obj, info)
            underlying = [];
            if ~isempty(info.underlying)
                underlying = obj.createConnectionInfo(info.underlying);
            end

            switch info.type
                case 'tcp'
                    r = Ice.TCPConnectionInfo(underlying, info.incoming, info.adapterName, info.connectionId, ...
                                              info.localAddress, info.localPort, info.remoteAddress, ...
                                              info.remotePort, info.rcvSize, info.sndSize);

                case 'ssl'
                    r = Ice.IPConnectionInfo(underlying, info.incoming, info.adapterName, info.connectionId, ...
                                             info.localAddress, info.localPort, info.remoteAddress, info.remotePort);

                case 'udp'
                    r = Ice.UDPConnectionInfo(underlying, info.incoming, info.adapterName, info.connectionId, ...
                                              info.localAddress, info.localPort, info.remoteAddress, ...
                                              info.remotePort, info.mcastAddress, info.mcastPort, ...
                                              info.rcvSize, info.sndSize);

                case 'ws'
                    r = Ice.WSConnectionInfo(underlying, info.incoming, info.adapterName, info.connectionId, ...
                                             info.headers);

                case 'ip'
                    r = Ice.IPConnectionInfo(underlying, info.incoming, info.adapterName, info.connectionId, ...
                                             info.localAddress, info.localPort, info.remoteAddress, info.remotePort);

                otherwise
                    r = Ice.ConnectionInfo(underlying, info.incoming, info.adapterName, info.connectionId);
            end
        end
    end

    properties(Access=private)
        communicator % The communicator wrapper
    end
end
