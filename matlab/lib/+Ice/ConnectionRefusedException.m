% ConnectionRefusedException   Summary of ConnectionRefusedException
%
% This exception indicates a connection failure for which the server host actively refuses a connection.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef ConnectionRefusedException < Ice.ConnectFailedException
    methods
        function obj = ConnectionRefusedException(errID, msg, error)
            if nargin <= 2
                error = 0;
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:ConnectionRefusedException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.ConnectionRefusedException';
            end
            obj = obj@Ice.ConnectFailedException(errID, msg, error);
        end
    end
end
