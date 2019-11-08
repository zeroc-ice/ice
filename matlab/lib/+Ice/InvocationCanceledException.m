% InvocationCanceledException   Summary of InvocationCanceledException
%
% This exception indicates that an asynchronous invocation failed
% because it was canceled explicitly by the user.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef InvocationCanceledException < Ice.LocalException
    methods
        function obj = InvocationCanceledException(ice_exid, ice_exmsg)
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:InvocationCanceledException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.InvocationCanceledException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
        end
        function id = ice_id(~)
            id = '::Ice::InvocationCanceledException';
        end
    end
end
