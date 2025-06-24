classdef (Sealed) EndpointInfo < Ice.EndpointInfo
    %ENDPOINTINFO Provides access to SSL endpoint information.

    % Copyright (c) ZeroC, Inc.

    methods(Hidden)
        function obj = EndpointInfo(underlying)
            assert(nargin == 1, 'Invalid number of arguments');
            obj@Ice.EndpointInfo(underlying);
        end
    end
end
