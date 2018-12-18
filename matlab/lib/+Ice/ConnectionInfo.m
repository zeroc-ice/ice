classdef ConnectionInfo < handle
    % ConnectionInfo   Base class providing access to the connection details.
    %
    % ConnectionInfo Properties:
    %   underlying (Ice.ConnectionInfo) - The information of the underyling
    %     transport or an empty array if there's no underlying transport.
    %   incoming (logical) - Whether or not the connection is an incoming or
    %     outgoing connection.
    %   adapterName (char) - The name of the adapter associated with the
    %     connection.
    %   connectionId (char) - The connection id.

    % Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

    methods
        function obj = ConnectionInfo(underlying, incoming, adapterName, connectionId)
            if nargin == 0
                underlying = [];
                incoming = false;
                adapterName = '';
                connectionId = '';
            end
            obj.underlying = underlying;
            obj.incoming = incoming;
            obj.adapterName = adapterName;
            obj.connectionId = connectionId;
        end
    end
    properties(SetAccess=private)
        % underlying   The information of the underyling transport or null
        %   if there's no underlying transport.
        underlying

        % incoming   Whether or not the connection is an incoming or outgoing
        %   connection.
        incoming logical

        % adapter   The name of the adapter associated with the connection.
        adapterName char

        % connectionId   The connection id.
        connectionId char
    end
end
