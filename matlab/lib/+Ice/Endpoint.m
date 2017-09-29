%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Endpoint < IceInternal.WrapperObject
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
                r = obj.callWithResult_('equals', other.impl_);
            end
        end
        function r = toString(obj)
            r = obj.callWithResult_('toString');
        end
        function r = getInfo(obj)
            info = obj.callWithResult_('getInfo');
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
                r = Ice.OpaqueEndpointInfo(underlying, info.timeout, info.compress, info.rawEncoding, info.rawBytes);
            else
                switch info.type
                    case Ice.TCPEndpointType.value
                        r = Ice.TCPEndpointInfo(underlying, info.timeout, info.compress, info.host, info.port, ...
                                                info.sourceAddress);

                    case Ice.SSLEndpointType.value
                        r = Ice.IPEndpointInfo(info.type, info.datagram, info.secure, underlying, info.timeout, ...
                                               info.compress, info.host, info.port, info.sourceAddress);

                    case Ice.UDPEndpointType.value
                        r = Ice.UDPEndpointInfo(underlying, info.timeout,  info.compress, info.host, info.port, ...
                                                info.sourceAddress, info.mcastInterface, info.mcastTtl);

                    case Ice.WSEndpointType.value
                        r = Ice.WSEndpointInfo(info.secure, underlying, info.timeout,  info.compress, info.resource);

                    case Ice.WSSEndpointType.value
                        r = Ice.WSEndpointInfo(info.secure, underlying, info.timeout,  info.compress, info.resource);

                    otherwise
                        r = Ice.EndpointInfo(info.type, info.datagram, info.secure, underlying, info.timeout, ...
                                             info.compress);
                end
            end
        end
    end
end
