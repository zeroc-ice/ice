% SocketException   Summary of SocketException
%
% This exception indicates socket errors.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef SocketException < Ice.SyscallException
    methods
        function obj = SocketException(errID, msg, error)
            if nargin <= 2
                error = 0;
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:SocketException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.SocketException';
            end
            obj = obj@Ice.SyscallException(errID, msg, error);
        end
    end
end
