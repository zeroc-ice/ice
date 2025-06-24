classdef (Sealed) WSConnectionInfo < Ice.ConnectionInfo
    %WSCONNECTIONINFO Provides access to the connection details of a WebSocket connection.
    %
    %   WSConnectionInfo Properties:
    %     headers - The headers from the HTTP upgrade request.

    % Copyright (c) ZeroC, Inc.

    methods(Hidden)
        function obj = WSConnectionInfo(underlying, headers)
            assert(nargin == 2, 'Invalid number of arguments');
            obj@Ice.ConnectionInfo(underlying);
            obj.headers = headers;
        end
    end
    properties(SetAccess=immutable)
        %HEADERS The headers from the HTTP upgrade request.
        %   dictionary(string, string)
        headers (1, 1) dictionary {Ice.mustBeStringStringDictionary} = configureDictionary('string', 'string')
    end
end
