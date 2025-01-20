classdef ConnectionInfo < handle
    % ConnectionInfo   Base class providing access to the connection details.
    %
    % ConnectionInfo Properties:
    %   underlying (Ice.ConnectionInfo) - The information of the underlying transport or an empty array if there's no
    %     underlying transport.
    %   connectionId (char) - The connection id.

    %  Copyright (c) ZeroC, Inc.

    properties(SetAccess=immutable)
        % underlying   The information of the underlying transport or an empty array if there's no underlying transport.
        underlying

        % connectionId   The connection id.
        connectionId char
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
