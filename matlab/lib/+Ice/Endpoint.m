classdef Endpoint < IceInternal.WrapperObject
    %ENDPOINT Specifies the address of the server-end of an Ice connection: an object adapter listens on one or more
    %   endpoints and a client establishes a connection to an endpoint.
    %
    %   Endpoint Methods:
    %     eq - Compares this Endpoint with another Endpoint for equality.
    %     getInfo - Returns the endpoint information.
    %     toString - Returns a string representation of the endpoint.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden, Access = {?Ice.Connection, ?Ice.ObjectPrx})
        function obj = Endpoint(impl)
            assert(isa(impl, 'lib.pointer'));
            obj@IceInternal.WrapperObject(impl);
        end
    end
    methods
        function r = eq(obj, other)
            %EQ Compares this Endpoint with another Endpoint for equality.
            %
            %   See also eq.
            if isempty(other) || ~isa(other, 'Ice.Endpoint')
                r = false;
            else
                %
                % Call into C++ to compare the two objects.
                %
                r = obj.iceCallWithResult('equals', other.impl_);
            end
        end

        function r = toString(obj)
            %TOSTRING Returns a string representation of the endpoint.
            %
            %   Output Arguments
            %     The string representation of the endpoint.
            %       character vector

            arguments
                obj (1, 1) Ice.Endpoint
            end
            r = obj.iceCallWithResult('toString');
        end

        function r = getInfo(obj)
            %GETINFO Returns the endpoint information.
            %
            %   Output Arguments
            %     r - The endpoint information.
            %       Ice.EndpointInfo scalar

            arguments
                obj (1, 1) Ice.Endpoint
            end
            info = obj.iceCallWithResult('getInfo');
            r = obj.createEndpointInfo(info);
        end
    end
    methods(Access=private)
        function r = createEndpointInfo(obj, info)
            underlying = Ice.EndpointInfo.empty;
            if ~isempty(info.underlying)
                underlying = obj.createEndpointInfo(info.underlying);
            end

            if ~isempty(info.rawEncoding)
                r = Ice.OpaqueEndpointInfo(info.type, info.rawEncoding, info.rawBytes);
            else
                % info.infoType points to the XxxEndpointInfo type we need to create
                switch info.infoType
                    case Ice.TCPEndpointType.value
                        r = Ice.TCPEndpointInfo(info.timeout, info.compress, info.host, info.port, ...
                                                info.sourceAddress, info.type, info.secure);

                    case Ice.SSLEndpointType.value
                        r = Ice.SSL.EndpointInfo(underlying);

                    case Ice.UDPEndpointType.value
                        r = Ice.UDPEndpointInfo(info.compress, info.host, info.port, info.sourceAddress, ...
                                                info.mcastInterface, info.mcastTtl);

                    case {Ice.WSEndpointType.value, Ice.WSSEndpointType.value}
                        r = Ice.WSEndpointInfo(underlying, info.resource);

                    otherwise
                        assert(false, 'unknown endpoint type');
                end
            end
        end
    end
end
