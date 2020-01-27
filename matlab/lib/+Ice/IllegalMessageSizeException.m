% IllegalMessageSizeException   Summary of IllegalMessageSizeException
%
% This exception indicates that a message size is less
% than the minimum required size.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef IllegalMessageSizeException < Ice.ProtocolException
    methods
        function obj = IllegalMessageSizeException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:IllegalMessageSizeException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.IllegalMessageSizeException';
            end
            obj = obj@Ice.ProtocolException(ice_exid, ice_exmsg, reason);
        end
        function id = ice_id(~)
            id = '::Ice::IllegalMessageSizeException';
        end
    end
end
