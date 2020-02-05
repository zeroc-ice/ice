% ConnectFailedException   Summary of ConnectFailedException
%
% This exception indicates connection failures.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef ConnectFailedException < Ice.SocketException
    methods
        function obj = ConnectFailedException(ice_exid, ice_exmsg, error)
            if nargin <= 2
                error = 0;
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:ConnectFailedException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.ConnectFailedException';
            end
            obj = obj@Ice.SocketException(ice_exid, ice_exmsg, error);
        end
        function id = ice_id(~)
            id = '::Ice::ConnectFailedException';
        end
    end
end
