% FixedProxyException   Summary of FixedProxyException
%
% This exception indicates that an attempt has been made to
% change the connection properties of a fixed proxy.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef FixedProxyException < Ice.LocalException
    methods
        function obj = FixedProxyException(ice_exid, ice_exmsg)
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:FixedProxyException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.FixedProxyException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
        end
        function id = ice_id(~)
            id = '::Ice::FixedProxyException';
        end
    end
end
