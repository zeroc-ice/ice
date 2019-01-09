classdef WSConnectionInfo < Ice.ConnectionInfo
    % WSConnectionInfo   Summary of WSConnectionInfo
    %
    % Provides access to the connection details of a WebSocket connection.
    %
    % WSConnectionInfo Properties:
    %   headers - The headers from the HTTP upgrade request.

    % Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

    methods
        function obj = WSConnectionInfo(underlying, incoming, adapterName, connectionId, headers)
            if nargin == 0
                underlying = [];
                incoming = false;
                adapterName = '';
                connectionId = '';
                headers = containers.Map('KeyType', 'char', 'ValueType', 'char');
            end
            obj = obj@Ice.ConnectionInfo(underlying, incoming, adapterName, connectionId);
            obj.headers = headers;
        end
    end
    properties(SetAccess=private)
        % headers - The headers from the HTTP upgrade request.
        headers
    end
end
