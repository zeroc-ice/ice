% ConnectionLostException   Summary of ConnectionLostException
%
% This exception indicates a lost connection.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef ConnectionLostException < Ice.SocketException
    methods
        function obj = ConnectionLostException(ice_exid, ice_exmsg, error)
            if nargin <= 2
                error = 0;
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:ConnectionLostException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.ConnectionLostException';
            end
            obj = obj@Ice.SocketException(ice_exid, ice_exmsg, error);
        end
        function id = ice_id(~)
            id = '::Ice::ConnectionLostException';
        end
    end
end
