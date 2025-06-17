classdef Endpoint < IceInternal.WrapperObject
    % Endpoint   Summary of Endpoint
    %
    % The user-level interface to an endpoint.
    %
    % Endpoint Methods:
    %   toString - Return a string representation of the endpoint.
    %   getInfo - Returns the endpoint information.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden, Access = {?Ice.Connection, ?Ice.ObjectPrx})
        function obj = Endpoint(impl)
            assert(isa(impl, 'lib.pointer'));
            obj@IceInternal.WrapperObject(impl);
        end
    end
    methods
        %
        % Override == operator.
        %
        function r = eq(obj, other)
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
            % toString   Return a string representation of the endpoint.
            %
            % Returns (char) - The string representation of the endpoint.

            r = obj.iceCallWithResult('toString');
        end
        function r = getInfo(obj)
            % getInfo   Returns the endpoint information.
            %
            % Returns (Ice.EndpointInfo) - The endpoint information class.

            info = obj.iceCallWithResult('getInfo');
            r = obj.createEndpointInfo(info);
        end
    end
    methods(Access=private)
        function r = createEndpointInfo(obj, info)
            underlying = [];
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
