% ConnectionIdleException   Summary of ConnectionIdleException
%
% This exception indicates that a connection was aborted by the idle check.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef ConnectionIdleException < Ice.LocalException
    methods
        function obj = ConnectionIdleException(ice_exid, ice_exmsg)
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:ConnectionIdleException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.ConnectionIdleException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
        end
        function id = ice_id(~)
            id = '::Ice::ConnectionIdleException';
        end
    end
end
