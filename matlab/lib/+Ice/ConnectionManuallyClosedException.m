% ConnectionManuallyClosedException   Summary of ConnectionManuallyClosedException
%
% This exception is raised by an operation call if the application closes the connection locally using
% Connection.close.
%
% ConnectionManuallyClosedException Properties:
%   graceful - True if the connection was closed gracefully, false otherwise.
%
% See also Connection.close

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef ConnectionManuallyClosedException < Ice.LocalException
    properties
        % graceful - True if the connection was closed gracefully, false otherwise.
        graceful logical
    end
    methods
        function obj = ConnectionManuallyClosedException(errID, msg, graceful)
            if nargin <= 2
                graceful = false;
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:ConnectionManuallyClosedException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.ConnectionManuallyClosedException';
            end
            obj = obj@Ice.LocalException(errID, msg);
            obj.graceful = graceful;
        end
    end
end
