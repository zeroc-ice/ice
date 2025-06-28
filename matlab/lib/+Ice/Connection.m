classdef Connection < IceInternal.WrapperObject
    %CONNECTION Represents a connection that uses the Ice protocol.
    %
    %   Connection Methods:
    %     abort - Aborts this connection.
    %     close - Closes the connection gracefully after waiting for all outstanding invocations to complete.
    %     createProxy - Creates a proxy that uses this connection.
    %     eq - Compares this connection with another Connection for equality.
    %     flushBatchRequests - Flushes any pending batch requests for this connection.
    %     flushBatchRequestsAsync - An asynchronous flushBatchRequests.
    %     getEndpoint - Gets the endpoint from which this connection was created.
    %     getInfo - Gets the connection information for this connection.
    %     setBufferSize - Sets the buffer sizes for this connection.
    %     throwException - Manually throws an exception to indicate that the connection is lost.
    %     toString - Gets a description of this connection.
    %     type - Gets the type of this connection.

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
            %   See also eq.

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
            %CLOSE Closes the connection gracefully after waiting for all outstanding invocations to complete.
            %
            %   Output Arguments
            %     f - A future that completes when the connection is closed.
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
            %CREATEPROXY Creates a special proxy that always uses this connection.
            %
            %   Input Arguments
            %     id - The identity for which a proxy is to be created.
            %       Ice.Identity scalar
            %
            %   Output Arguments
            %     r - A proxy that matches the given identity and uses this connection.
            %       Ice.ObjectPrx scalar

            arguments
                obj (1, 1) Ice.Connection
                id (1, 1) Ice.Identity
            end
            proxy = libpointer('voidPtr');
            obj.iceCall('createProxy', id, proxy);
            r = Ice.ObjectPrx(obj.communicator, '', proxy, obj.communicator.getEncoding());
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
            %TYPE Returns the connection type. This corresponds to the endpoint type, i.e., "tcp", "udp", etc.
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
            %   messages.
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
            %GETINFO Returns the connection information.
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
            %SETBUFFERSIZE Sets the connection buffer receive/send size.
            %
            %   Input Arguments
            %     rcvSize - The connection receive buffer size.
            %       int32 scalar
            %     sndSize - The connection send buffer size.
            %       int32 scalar

            arguments
                obj (1, 1) Ice.Connection
                rcvSize (1, 1) int32
                sndSize (1, 1) int32
            end
            obj.iceCall('setBufferSize', rcvSize, sndSize);
        end

        function throwException(obj)
            %THROWEXCEPTION Throws an exception indicating the reason for connection closure.
            %   For example, CloseConnectionException is thrown if the connection was closed gracefully, whereas
            %   ConnectionAbortedException/ConnectionClosedException are thrown if the connection was manually closed by
            %   the application. This method does nothing if the connection is not yet closed.

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
