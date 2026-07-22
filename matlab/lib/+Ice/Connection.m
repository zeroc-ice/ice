classdef Connection < IceInternal.WrapperObject
    %CONNECTION Represents a connection that uses the Ice protocol.
    %
    %   Connection Methods:
    %     abort - Aborts this connection.
    %     close - Starts a graceful closure of this connection once all outstanding invocations have completed.
    %     createProxy - Creates a special proxy (a "fixed proxy") that always uses this connection.
    %     disableInactivityCheck - Disables the inactivity check on this connection.
    %     eq - Compares this connection with another Connection for equality.
    %     flushBatchRequests - Flushes any pending batch requests for this connection.
    %     flushBatchRequestsAsync - An asynchronous flushBatchRequests.
    %     getEndpoint - Gets the endpoint from which this connection was created.
    %     getInfo - Returns the connection information.
    %     setBufferSize - Sets the size of the receive and send buffers.
    %     throwException - Throws the exception that provides the reason for the closure of this connection.
    %     toString - Returns a description of the connection as human readable text, suitable for logging or error
    %       messages.
    %     type - Returns the connection type.

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
            %EQ Compares this connection with another Connection for equality.
            %
            %   Output Arguments
            %     r - True if both objects refer to the same connection, false otherwise.
            %       logical scalar

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
            %ABORT Aborts this connection.

            arguments
                obj (1, 1) Ice.Connection
            end
            obj.iceCall('abort');
        end

        function f = close(obj)
            %CLOSE Starts a graceful closure of this connection once all outstanding invocations have completed.
            %
            %   Output Arguments
            %     f - A future that completes when the connection closure completes. If closing takes longer than the
            %       configured close timeout, the connection is aborted with an Ice.CloseTimeoutException and the
            %       future completes with this exception.
            %       Ice.Future scalar

            arguments
                obj (1, 1) Ice.Connection
            end
            future = libpointer('voidPtr');
            obj.iceCall('close', future);
            assert(~isNull(future));
            f = Ice.Future(future, 'close', 0, 'Ice_SimpleFuture', @(fut) fut.iceCall('check'));
        end

        function r = createProxy(obj, id)
            %CREATEPROXY Creates a special proxy (a "fixed proxy") that always uses this connection.
            %
            %   Input Arguments
            %     id - The identity of the target object.
            %       Ice.Identity scalar
            %
            %   Output Arguments
            %     r - A fixed proxy with the provided identity.
            %       Ice.ObjectPrx scalar
            %
            %   Exceptions
            %     Ice.CommunicatorDestroyedException - If the communicator has been destroyed.

            arguments
                obj (1, 1) Ice.Connection
                id (1, 1) Ice.Identity
            end
            proxy = libpointer('voidPtr');
            obj.iceCall('createProxy', id, proxy);
            r = Ice.ObjectPrx(obj.communicator, '', proxy, obj.communicator.getEncoding());
        end

        function disableInactivityCheck(obj)
            %DISABLEINACTIVITYCHECK Disables the inactivity check on this connection.

            arguments
                obj (1, 1) Ice.Connection
            end
            obj.iceCall('disableInactivityCheck');
        end

        function r = getEndpoint(obj)
            %GETENDPOINT Gets the endpoint from which the connection was created.
            %
            %   Output Arguments
            %     r - The endpoint from which the connection was created.
            %       Ice.Endpoint scalar

            arguments
                obj (1, 1) Ice.Connection
            end
            endpoint = libpointer('voidPtr');
            obj.iceCall('getEndpoint', endpoint);
            r = Ice.Endpoint(endpoint);
        end

        function flushBatchRequests(obj, compress)
            %FLUSHBATCHREQUESTS Flushes any pending batch requests for this connection. This means all batch requests
            %   invoked on fixed proxies associated with the connection.
            %
            %   Input Arguments
            %     compress - Specifies whether or not the queued batch requests should be compressed before being sent
            %       over the wire.
            %       Ice.CompressBatch scalar
            %
            %   Exceptions
            %     Ice.LocalException - If the flush fails. For example, this method throws an
            %       Ice.CommunicatorDestroyedException if the communicator has been destroyed.

            arguments
                obj (1, 1) Ice.Connection
                compress (1, 1) Ice.CompressBatch
            end
            obj.iceCall('flushBatchRequests', compress);
        end

        function r = flushBatchRequestsAsync(obj, compress)
            %FLUSHBATCHREQUESTSASYNC Flushes any pending batch requests for this connection. This means all batch
            %   requests invoked on fixed proxies associated with the connection.
            %
            %   Input Arguments
            %     compress - Specifies whether or not the queued batch requests should be compressed before being sent
            %       over the wire.
            %       Ice.CompressBatch scalar
            %
            %   Output Arguments
            %     r - A future that will be completed when the invocation completes.
            %       Ice.Future scalar
            %
            %   Exceptions
            %     Ice.CommunicatorDestroyedException - If the communicator has been destroyed. This exception is
            %       thrown synchronously.

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
            %TYPE Returns the connection type. This corresponds to the endpoint type, such as "tcp", "udp", etc.
            %
            %   Output Arguments
            %     r - The type of the connection.
            %       character vector

            arguments
                obj (1, 1) Ice.Connection
            end
            r = obj.iceCallWithResult('type');
        end

        function r = toString(obj)
            %TOSTRING Returns a description of the connection as human readable text, suitable for logging or error
            %   messages. This method remains usable after the connection is closed or aborted.
            %
            %   Output Arguments
            %     r - The description of the connection as human readable text.
            %       character vector

            arguments
                obj (1, 1) Ice.Connection
            end
            r = obj.iceCallWithResult('toString');
        end

        function r = getInfo(obj)
            %GETINFO Returns the connection information. If the connection is closed, this method throws the exception
            %   that caused the closure.
            %
            %   Output Arguments
            %     r - The connection information.
            %       Ice.ConnectionInfo scalar

            arguments
                obj (1, 1) Ice.Connection
            end
            info = obj.iceCallWithResult('getInfo');
            r = obj.createConnectionInfo(info);
        end

        function setBufferSize(obj, rcvSize, sndSize)
            %SETBUFFERSIZE Sets the size of the receive and send buffers.
            %
            %   Input Arguments
            %     rcvSize - The size of the receive buffer.
            %       int32 scalar
            %     sndSize - The size of the send buffer.
            %       int32 scalar

            arguments
                obj (1, 1) Ice.Connection
                rcvSize (1, 1) int32
                sndSize (1, 1) int32
            end
            obj.iceCall('setBufferSize', rcvSize, sndSize);
        end

        function throwException(obj)
            %THROWEXCEPTION Throws the exception that provides the reason for the closure of this connection.
            %   This method does nothing if the connection is not yet closing or closed. For example, this method
            %   throws Ice.CloseConnectionException when the connection was closed gracefully by the peer,
            %   Ice.ConnectionClosedException when the connection was closed gracefully by the application, and
            %   Ice.ConnectionAbortedException when the connection was aborted with abort.

            arguments
                obj (1, 1) Ice.Connection
            end
            obj.iceCall('throwException');
        end
    end

    methods (Access = private)
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

    properties (Access = private)
        communicator % The communicator wrapper
    end
end
