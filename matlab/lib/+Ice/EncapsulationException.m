% EncapsulationException   Summary of EncapsulationException
%
% This exception indicates a malformed data encapsulation.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef EncapsulationException < Ice.MarshalException
    methods
        function obj = EncapsulationException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:EncapsulationException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.EncapsulationException';
            end
            obj = obj@Ice.MarshalException(ice_exid, ice_exmsg, reason);
        end
        function id = ice_id(~)
            id = '::Ice::EncapsulationException';
        end
    end
end
