% SyscallException   Summary of SyscallException
%
% This exception is raised if a system error occurred in the server
% or client process. There are many possible causes for such a system
% exception. For details on the cause, SyscallException.error
% should be inspected.
%
% SyscallException Properties:
%   error - The error number describing the system exception.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef SyscallException < Ice.LocalException
    properties
        % error - The error number describing the system exception. For C++ and
        % Unix, this is equivalent to errno. For C++
        % and Windows, this is the value returned by
        % GetLastError() or
        % WSAGetLastError().
        error int32
    end
    methods
        function obj = SyscallException(ice_exid, ice_exmsg, error)
            if nargin <= 2
                error = 0;
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:SyscallException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.SyscallException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.error = error;
        end
        function id = ice_id(~)
            id = '::Ice::SyscallException';
        end
    end
end
