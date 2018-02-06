classdef Endpoint < IceInternal.WrapperObject
    % Endpoint   Summary of Endpoint
    %
    % The user-level interface to an endpoint.
    %
    % Endpoint Methods:
    %   toString - Return a string representation of the endpoint.
    %   getInfo - Returns the endpoint information.

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

    methods
        function obj = Endpoint(impl)
            if ~isa(impl, 'lib.pointer')
                throw(MException('Ice:ArgumentException', 'invalid argument'));
            end
            obj = obj@IceInternal.WrapperObject(impl);
        end
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
                r = Ice.OpaqueEndpointInfo(info.type, underlying, info.timeout, info.compress, info.rawEncoding, ...
                                           info.rawBytes);
            else
                switch info.infoType
                    case Ice.TCPEndpointType.value
                        r = Ice.TCPEndpointInfo(info.type, info.secure, underlying, info.timeout, info.compress, ...
                                                info.host, info.port, info.sourceAddress);

                    case Ice.SSLEndpointType.value
                        r = IceSSL.EndpointInfo(info.type, info.secure, underlying, info.timeout, info.compress);

                    case Ice.UDPEndpointType.value
                        r = Ice.UDPEndpointInfo(info.type, underlying, info.timeout,  info.compress, info.host, ...
                                                info.port, info.sourceAddress, info.mcastInterface, info.mcastTtl);

                    case {Ice.WSEndpointType.value, Ice.WSSEndpointType.value}
                        r = Ice.WSEndpointInfo(info.type, info.secure, underlying, info.timeout,  info.compress, ...
                                               info.resource);

                    otherwise
                        r = Ice.EndpointInfo(info.type, info.datagram, info.secure, underlying, info.timeout, ...
                                             info.compress);
                end
            end
        end
    end
end
