classdef EndpointInfo
    %ENDPOINTINFO Base class for the endpoint info classes.
    %
    %   EndpointInfo Properties:
    %     underlying - The EndpointInfo of the underlying endpoint, if any.
    %     compress - Specifies whether or not compression should be used if available when using this endpoint.
    %
    %   EndpointInfo Methods:
    %     type - Returns the type of the endpoint.
    %     datagram - Returns true if this endpoint is a datagram endpoint.
    %     secure - Returns true if this endpoint is a secure endpoint.

    % Copyright (c) ZeroC, Inc.

    methods
        function r = type(obj)
            %TYPE Returns the type of the endpoint.
            %
            %   Output Arguments
            %     r - The endpoint type.
            %       int16 scalar

            arguments
                obj (1, 1) Ice.EndpointInfo
            end
            if ~isempty(obj.underlying)
                r = obj.underlying.type();
            else
                r = -1;
            end
        end

        function r = datagram(obj)
            %DATAGRAM Returns true if this endpoint is a datagram endpoint.
            %
            %   Output Arguments
            %     r - True for a datagram endpoint.
            %       logical scalar

            arguments
                obj (1, 1) Ice.EndpointInfo
            end
            if ~isempty(obj.underlying)
                r = obj.underlying.datagram();
            else
                r = false;
            end
        end

        function r = secure(obj)
            %SECURE Returns true if this endpoint is a secure endpoint.
            %
            %   Output Arguments
            %     r - True for a secure endpoint.
            %       logical scalar

            arguments
                obj (1, 1) Ice.EndpointInfo
            end
            if ~isempty(obj.underlying)
                r = obj.underlying.secure();
            else
                r = false;
            end
        end
    end

    properties (SetAccess = immutable)
        %UNDERLYING The EndpointInfo of the underlying endpoint, if any.
        %   Ice.EndpointInfo scalar | empty array of Ice.EndpointInfo
        underlying Ice.EndpointInfo {mustBeScalarOrEmpty}

        %COMPRESS Specifies whether or not compression should be used if available when using this endpoint.
        %   logical scalar
        compress (1, 1) logical
    end
    methods (Hidden, Access = protected)
        function obj = EndpointInfo(underlying, compress)
            if nargin == 1
                assert(~isempty(underlying), 'underlying cannot be empty');
                compress = underlying.compress;
            else
                assert(isempty(underlying), 'underlying must be empty');
                assert(nargin == 2, 'Invalid number of arguments');
            end
            obj.underlying = underlying;
            obj.compress = compress;
        end
    end
end
