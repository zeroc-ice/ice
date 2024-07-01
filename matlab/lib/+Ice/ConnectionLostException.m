% ConnectionLostException   Summary of ConnectionLostException
%
% This exception indicates a lost connection.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef ConnectionLostException < Ice.SocketException
    methods
        function obj = ConnectionLostException(errID, msg, error)
            if nargin <= 2
                error = 0;
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:ConnectionLostException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.ConnectionLostException';
            end
            obj = obj@Ice.SocketException(errID, msg, error);
        end
    end
end
