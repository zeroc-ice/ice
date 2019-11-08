% OperationInterruptedException   Summary of OperationInterruptedException
%
% This exception indicates a request was interrupted.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef OperationInterruptedException < Ice.LocalException
    methods
        function obj = OperationInterruptedException(ice_exid, ice_exmsg)
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:OperationInterruptedException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.OperationInterruptedException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
        end
        function id = ice_id(~)
            id = '::Ice::OperationInterruptedException';
        end
    end
end
