classdef EndpointInfo < handle
    % EndpointInfo   Summary of EndpointInfo
    %
    % Base class providing access to the endpoint details.
    %
    % EndpointInfo Methods:
    %   type - Returns the type of the endpoint.
    %   datagram - Returns true if this endpoint is a datagram endpoint.
    %   secure - Returns true if this endpoint is a secure endpoint.
    %
    % EndpointInfo Properties:
    %   underlying (Ice.EndpointInfo) - The information of the underlying endpoint or an empty array if there's no
    %   underlying endpoint.
    %   timeout (int32) - The timeout for the endpoint in milliseconds.
    %   compress (logical) - Specifies whether or not compression should be used if available when using this endpoint.

    % Copyright (c) ZeroC, Inc.

    methods
        function r = type(obj)
            % type   Returns the type of the endpoint.
            %
            % Returns (int16) - The endpoint type.

            if ~isempty(obj.underlying)
                r = obj.underlying.type();
            else
                r = -1;
            end
        end
        function r = datagram(obj)
            % datagram   Returns true if this endpoint is a datagram endpoint.
            %
            % Returns (logical) - True for a datagram endpoint.

            if ~isempty(obj.underlying)
                r = obj.underlying.datagram();
            else
                r = false;
            end
        end
        function r = secure(obj)
            % secure   Returns true if this endpoint is a secure endpoint.
            %
            % Returns (logical) - True for a secure endpoint.

            if ~isempty(obj.underlying)
                r = obj.underlying.secure();
            else
                r = false;
            end
        end
    end
    properties(SetAccess=immutable)
        % underlying   The information of the underlying endpoint or an empty array if there's no underlying endpoint.
        underlying

        % timeout   The timeout for the endpoint in milliseconds. 0 means non-blocking, -1 means no timeout.
        timeout int32

        % compress   Specifies whether or not compression should be used if available when using this endpoint.
        compress logical
    end
    methods(Access=protected)
        function obj = EndpointInfo(underlying, timeout, compress)
            if nargin == 1
                assert(~isempty(underlying), 'underlying cannot be empty');
                timeout = underlying.timeout;
                compress = underlying.compress;
            else
                assert(isempty(underlying), 'underlying must be empty');
                assert(nargin == 3, 'Invalid number of arguments');
            end
            obj.underlying = underlying;
            obj.timeout = timeout;
            obj.compress = compress;
        end
    end
end
