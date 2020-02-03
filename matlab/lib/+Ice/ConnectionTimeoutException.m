% ConnectionTimeoutException   Summary of ConnectionTimeoutException
%
% This exception indicates that a connection has been shut down because it has been
% idle for some time.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef ConnectionTimeoutException < Ice.TimeoutException
    methods
        function obj = ConnectionTimeoutException(ice_exid, ice_exmsg)
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:ConnectionTimeoutException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.ConnectionTimeoutException';
            end
            obj = obj@Ice.TimeoutException(ice_exid, ice_exmsg);
        end
        function id = ice_id(~)
            id = '::Ice::ConnectionTimeoutException';
        end
    end
end
