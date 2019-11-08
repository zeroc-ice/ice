% ConnectionRefusedException   Summary of ConnectionRefusedException
%
% This exception indicates a connection failure for which
% the server host actively refuses a connection.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef ConnectionRefusedException < Ice.ConnectFailedException
    methods
        function obj = ConnectionRefusedException(ice_exid, ice_exmsg, error)
            if nargin <= 2
                error = 0;
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:ConnectionRefusedException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.ConnectionRefusedException';
            end
            obj = obj@Ice.ConnectFailedException(ice_exid, ice_exmsg, error);
        end
        function id = ice_id(~)
            id = '::Ice::ConnectionRefusedException';
        end
    end
end
