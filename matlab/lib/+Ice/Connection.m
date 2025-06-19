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
    %   type - Return the connection type.
    %   toString - Return a description of the connection as human readable
    %     text, suitable for logging or error messages.
    %   getInfo - Returns the connection information.
    %   setBufferSize - Set the connection buffer receive/send size.
    %   throwException - Throw an exception indicating the reason for
    %     connection closure.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden, Access = ?Ice.ObjectPrx)
        function obj = Connection(impl, communicator)
            assert(isa(impl, 'lib.pointer'));
            obj@IceInternal.WrapperObject(impl);
            obj.communicator = communicator;
        end
    end
    methods
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
        function abort(obj)
            % abort   Aborts this connection.
            %

            arguments
                obj (1, 1) Ice.Connection
            end
            obj.iceCall('abort');
        end
        function f = close(obj)
            % close   Closes the connection gracefully after waiting for all outstanding invocations to complete.
            %
            % Returns (Ice.Future) - A future that completes when the connnection is closed.

            arguments
                obj (1, 1) Ice.Connection
            end
            future = libpointer('voidPtr');
            obj.iceCall('close', future);
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

            arguments
                obj (1, 1) Ice.Connection
                id (1, 1) Ice.Identity
            end
            proxy = libpointer('voidPtr');
            obj.iceCall('createProxy', id, proxy);
            r = Ice.ObjectPrx(obj.communicator, '', proxy, obj.communicator.getEncoding());
        end
        function r = getEndpoint(obj)
            % getEndpoint   Get the endpoint from which the connection was
            %   created.
            %
            % Returns (Ice.Endpoint) - The endpoint from which the connection
            %   was created.

            arguments
                obj (1, 1) Ice.Connection
            end
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

            arguments
                obj (1, 1) Ice.Connection
                compress (1, 1) Ice.CompressBatch
            end
            obj.iceCall('flushBatchRequests', compress);
        end
        function r = flushBatchRequestsAsync(obj, compress)
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

            arguments
                obj (1, 1) Ice.Connection
                compress (1, 1) Ice.CompressBatch
            end
            future = libpointer('voidPtr');
            obj.iceCall('flushBatchRequestsAsync', compress, future);
            assert(~isNull(future));
            r = Ice.Future(future, 'flushBatchRequests', 0, 'Ice_SimpleFuture', @(fut) fut.iceCall('check'));
        end
        function r = type(obj)
            % type   Return the connection type. This corresponds to the
            %   endpoint type, i.e., "tcp", "udp", etc.
            %
            % Returns (char) - The type of the connection.

            arguments
                obj (1, 1) Ice.Connection
            end
            r = obj.iceCallWithResult('type');
        end
        function r = toString(obj)
            % toString   Return a description of the connection as human
            %   readable text, suitable for logging or error messages.
            %
            % Returns (char) - The description of the connection as human
            %   readable text.

            arguments
                obj (1, 1) Ice.Connection
            end
            r = obj.iceCallWithResult('toString');
        end
        function r = getInfo(obj)
            % getInfo   Returns the connection information.
            %
            % Returns (Ice.ConnectionInfo) - The connection information.

            arguments
                obj (1, 1) Ice.Connection
            end
            info = obj.iceCallWithResult('getInfo');
            r = obj.createConnectionInfo(info);
        end
        function setBufferSize(obj, rcvSize, sndSize)
            % setBufferSize   Set the connection buffer receive/send size.
            %
            % Parameters:
            %   rcvSize (int32) - The connection receive buffer size.
            %   sndSize (int32) - The connection send buffer size.

            arguments
                obj (1, 1) Ice.Connection
                rcvSize (1, 1) int32
                sndSize (1, 1) int32
            end
            obj.iceCall('setBufferSize', rcvSize, sndSize);
        end
        function throwException(obj)
            % throwException   Throw an exception indicating the reason for
            %   connection closure. For example, CloseConnectionException is
            %   raised if the connection was closed gracefully, whereas
            %   ConnectionAbortedException/ConnectionClosedException is raised
            %   if the connection was manually closed by the application. This
            %   operation does nothing if the connection is not yet closed.

            arguments
                obj (1, 1) Ice.Connection
            end
            obj.iceCall('throwException');
        end
    end

    methods(Access=private)
        function r = createConnectionInfo(obj, info)
            underlying = Ice.ConnectionInfo.empty;
            if ~isempty(info.underlying)
                underlying = obj.createConnectionInfo(info.underlying);
            end

            switch info.type
                case 'tcp'
                    r = Ice.TCPConnectionInfo(info.connectionId, info.localAddress, info.localPort, ...
                                              info.remoteAddress, info.remotePort, info.rcvSize, info.sndSize);

                case 'ssl'
                    r = Ice.SSL.ConnectionInfo(underlying, info.peerCertificate);

                case 'udp'
                    r = Ice.UDPConnectionInfo(info.connectionId, info.localAddress, info.localPort, ...
                                              info.remoteAddress, info.remotePort, ...
                                              info.mcastAddress, info.mcastPort, ...
                                              info.rcvSize, info.sndSize);

                case 'ws'
                    r = Ice.WSConnectionInfo(underlying, info.headers);

                otherwise
                    assert(false, 'unknown connection type');
            end
        end
    end

    properties(Access=private)
        communicator % The communicator wrapper
    end
end
