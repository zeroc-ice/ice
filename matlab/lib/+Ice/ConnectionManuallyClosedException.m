% ConnectionManuallyClosedException   Summary of ConnectionManuallyClosedException
%
% This exception is raised by an operation call if the application
% closes the connection locally using Connection.close.
%
% ConnectionManuallyClosedException Properties:
%   graceful - True if the connection was closed gracefully, false otherwise.
%
% See also Connection.close

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef ConnectionManuallyClosedException < Ice.LocalException
    properties
        % graceful - True if the connection was closed gracefully, false otherwise.
        graceful logical
    end
    methods
        function obj = ConnectionManuallyClosedException(ice_exid, ice_exmsg, graceful)
            if nargin <= 2
                graceful = false;
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:ConnectionManuallyClosedException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.ConnectionManuallyClosedException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.graceful = graceful;
        end
        function id = ice_id(~)
            id = '::Ice::ConnectionManuallyClosedException';
        end
    end
end
