classdef ConnectionInfo < handle
    %CONNECTIONINFO Base class for all connection info classes.

    % Copyright (c) ZeroC, Inc.

    properties(SetAccess=immutable)
        % underlying   The information of the underlying transport or an empty array if there's no underlying transport.
        underlying Ice.ConnectionInfo {mustBeScalarOrEmpty}

        % connectionId   The connection id.
        connectionId (1, :) char
    end
    methods(Access=protected)
        function obj = ConnectionInfo(underlying, connectionId)
            if nargin == 1
                assert(~isempty(underlying), 'underlying cannot be empty');
                connectionId = underlying.connectionId;
            else
                assert(nargin == 2, 'Invalid number of arguments');
                assert(isempty(underlying), 'underlying must be empty');
            end
            obj.underlying = underlying;
            obj.connectionId = connectionId;
        end
    end
end
