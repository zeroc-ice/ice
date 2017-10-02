%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef ConnectionInfo < handle
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
        underlying
        incoming logical
        adapterName char
        connectionId char
    end
end
