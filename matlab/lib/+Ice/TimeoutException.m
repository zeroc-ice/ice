% TimeoutException   Summary of TimeoutException
%
% This exception indicates a timeout condition.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef TimeoutException < Ice.LocalException
    methods
        function obj = TimeoutException(ice_exid, ice_exmsg)
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:TimeoutException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.TimeoutException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
        end
        function id = ice_id(~)
            id = '::Ice::TimeoutException';
        end
    end
end
