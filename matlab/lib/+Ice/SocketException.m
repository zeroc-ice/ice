% SocketException   Summary of SocketException
%
% This exception indicates socket errors.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef SocketException < Ice.SyscallException
    methods
        function obj = SocketException(ice_exid, ice_exmsg, error)
            if nargin <= 2
                error = 0;
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:SocketException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.SocketException';
            end
            obj = obj@Ice.SyscallException(ice_exid, ice_exmsg, error);
        end
        function id = ice_id(~)
            id = '::Ice::SocketException';
        end
    end
end
