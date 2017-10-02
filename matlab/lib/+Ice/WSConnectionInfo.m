%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef WSConnectionInfo < Ice.ConnectionInfo
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
        headers
    end
end
