% CommunicatorDestroyedException   Summary of CommunicatorDestroyedException
%
% This exception is raised if the Communicator has been destroyed.
%
% See also Communicator.destroy

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef CommunicatorDestroyedException < Ice.LocalException
    methods
        function obj = CommunicatorDestroyedException(ice_exid, ice_exmsg)
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:CommunicatorDestroyedException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.CommunicatorDestroyedException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
        end
        function id = ice_id(~)
            id = '::Ice::CommunicatorDestroyedException';
        end
    end
end
