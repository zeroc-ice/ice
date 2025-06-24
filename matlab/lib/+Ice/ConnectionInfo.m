classdef ConnectionInfo < handle
    %CONNECTIONINFO Base class for all connection info classes.
    %
    %   ConnectionInfo Properties:
    %     underlying - The information of the underlying transport or an empty array if there's no underlying transport.
    %     connectionId - The connection id.

    % Copyright (c) ZeroC, Inc.

    properties(SetAccess=immutable)
        %UNDERLYING The information of the underlying transport or an empty array if there's no underlying transport.
        %   Ice.ConnectionInfo scalar | empty array of Ice.ConnectionInfo
        underlying Ice.ConnectionInfo {mustBeScalarOrEmpty}

        %CONNECTIONID The connection id.
        %   character vector
        connectionId (1, :) char
    end
    methods(Hidden, Access=protected)
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
