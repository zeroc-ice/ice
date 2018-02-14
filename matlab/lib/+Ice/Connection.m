classdef Connection < IceInternal.WrapperObject
    % Connection   Summary of Connection
    %
    % The user-level interface to a connection.
    %
    % Connection Methods:
    %   close - Manually close the connection using the specified closure mode.
    %   closeAsync - Manually close the connection using the specified closure
    %     mode.
    %   createProxy - Create a special proxy that always uses this connection.
    %   getEndpoint - Get the endpoint from which the connection was created.
    %   flushBatchRequests - Flush any pending batch requests for this
    %     connection.
    %   flushBatchRequestsAsync - Flush any pending batch requests for this
    %     connection.
    %   heartbeat - Send a heartbeat message.
    %   heartbeatAsync - Send a heartbeat message.
    %   setACM - Set the active connection management parameters.
    %   getACM - Get the ACM parameters.
    %   type - Return the connection type.
    %   timeout - Get the timeout for the connection.
    %   toString - Return a description of the connection as human readable
    %     text, suitable for logging or error messages.
    %   getInfo - Returns the connection information.
    %   setBufferSize - Set the connection buffer receive/send size.
    %   throwException - Throw an exception indicating the reason for
    %     connection closure.

    % Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

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
            % close   Manually close the connection using the specified
            %   closure mode.
            %
            % Parameters:
            %   mode (Ice.ConnectionClose) - Determines how the connection
            %     will be closed.

            obj.iceCall('close', mode);
        end
        function f = closeAsync(obj)
            % closeAsync   Manually close the connection using the specified
            %   closure mode.
            %
            % Parameters:
            %   mode (Ice.ConnectionClose) - Determines how the connection
            %     will be closed.
            %
            % Returns (Ice.Future) - A future that will be completed when the
            %   invocation completes.

            future = libpointer('voidPtr');
            obj.iceCall('closeAsync', future);
            assert(~isNull(future));
            f = Ice.Future(future, 'close', 0, 'Ice_SimpleFuture', @(fut) fut.iceCall('check'));
        end
        function r = createProxy(obj, id)
            % createProxy   Create a special proxy that always uses this
            %   connection. This can be used for callbacks from a server to a
            %   client if the server cannot directly establish a connection to
            %   the client, for example because of firewalls. In this case,
            %   the server would create a proxy using an already established
            %   connection from the client.
            %
            % Parameters:
            %   id (Ice.Identity) - The identity for which a proxy is to be
            %     created.
            %
            % Returns (Ice.ObjectPrx) - A proxy that matches the given identity
            %   and uses this connection.

            proxy = libpointer('voidPtr');
            obj.iceCall('createProxy', id, proxy);
            r = Ice.ObjectPrx(obj.communicator, obj.communicator.getEncoding(), proxy);
        end
        function r = getEndpoint(obj)
            % getEndpoint   Get the endpoint from which the connection was
            %   created.
            %
            % Returns (Ice.Endpoint) - The endpoint from which the connection
            %   was created.

            endpoint = libpointer('voidPtr');
            obj.iceCall('getEndpoint', endpoint);
            r = Ice.Endpoint(endpoint);
        end
        function flushBatchRequests(obj, compress)
            % flushBatchRequests   Flush any pending batch requests for this
            %   connection. This means all batch requests invoked on fixed
            %   proxies associated with the connection.
            %
            % Parameters:
            %   compress (Ice.CompressBatch) - Specifies whether or not the
            %     queued batch requests should be compressed before being sent
            %     over the wire.

            obj.iceCall('flushBatchRequests', compress);
        end
        function r = flushBatchRequestsAsync(obj)
            % flushBatchRequestsAsync   Flush any pending batch requests for
            %   this connection. This means all batch requests invoked on fixed
            %   proxies associated with the connection.
            %
            % Parameters:
            %   compress (Ice.CompressBatch) - Specifies whether or not the
            %     queued batch requests should be compressed before being sent
            %     over the wire.
            %
            % Returns (Ice.Future) - A future that will be completed when the
            %   invocation completes.

            future = libpointer('voidPtr');
            obj.iceCall('flushBatchRequestsAsync', future);
            assert(~isNull(future));
            r = Ice.Future(future, 'flushBatchRequests', 0, 'Ice_SimpleFuture', @(fut) fut.iceCall('check'));
        end
        function heartbeat(obj)
            % heartbeat   Send a heartbeat message.

            obj.iceCall('heartbeat');
        end
        function r = heartbeatAsync(obj)
            % heartbeatAsync   Send a heartbeat message.
            %
            % Returns (Ice.Future) - A future that will be completed when the
            %   invocation completes.

            future = libpointer('voidPtr');
            obj.iceCall('heartbeatAsync', future);
            assert(~isNull(future));
            r = Ice.Future(future, 'heartbeat', 0, 'Ice_SimpleFuture', @(fut) fut.iceCall('check'));
        end
        function setACM(obj, timeout, close, heartbeat)
            % setACM   Set the active connection management parameters.
            %
            % Parameters:
            %   timeout (int32) - The timeout value in milliseconds.
            %   close (Ice.ACMClose) - The close condition.
            %   heartbeat (Ice.ACMHeartbeat) - The hertbeat condition.

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
            % getACM   Get the ACM parameters.
            %
            % Returns (Ice.ACM) - The ACM parameters.

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
            % type   Return the connection type. This corresponds to the
            %   endpoint type, i.e., "tcp", "udp", etc.
            %
            % Returns (char) - The type of the connection.

            r = obj.iceCallWithResult('type');
        end
        function r = timeout(obj)
            % timeout   Get the timeout for the connection.
            %
            % Returns (int32) - The connection's timeout.

            r = obj.iceCallWithResult('timeout');
        end
        function r = toString(obj)
            % toString   Return a description of the connection as human
            %   readable text, suitable for logging or error messages.
            %
            % Returns (char) - The description of the connection as human
            %   readable text.

            r = obj.iceCallWithResult('toString');
        end
        function r = getInfo(obj)
            % getInfo   Returns the connection information.
            %
            % Returns (Ice.ConnectionInfo) - The connection information.

            info = obj.iceCallWithResult('getInfo');
            r = obj.createConnectionInfo(info);
        end
        function setBufferSize(obj, rcvSize, sndSize)
            % setBufferSize   Set the connection buffer receive/send size.
            %
            % Parameters:
            %   rcvSize (int32) - The connection receive buffer size.
            %   sndSize (int32) - The connection send buffer size.

            obj.iceCall('setBufferSize', rcvSize, sndSize);
        end
        function throwException(obj)
            % throwException   Throw an exception indicating the reason for
            %   connection closure. For example, CloseConnectionException is
            %   raised if the connection was closed gracefully, whereas
            %   ConnectionManuallyClosedException is raised if the connection
            %   was manually closed by the application. This operation does
            %   nothing if the connection is not yet closed.

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
