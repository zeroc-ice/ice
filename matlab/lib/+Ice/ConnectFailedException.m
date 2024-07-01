% ConnectFailedException   Summary of ConnectFailedException
%
% This exception indicates connection failures.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef ConnectFailedException < Ice.SocketException
    methods
        function obj = ConnectFailedException(errID, msg, error)
            if nargin <= 2
                error = 0;
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:ConnectFailedException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.ConnectFailedException';
            end
            obj = obj@Ice.SocketException(errID, msg, error);
        end
    end
end
