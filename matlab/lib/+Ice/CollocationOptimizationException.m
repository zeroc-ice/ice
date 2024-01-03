% CollocationOptimizationException   Summary of CollocationOptimizationException
%
% This exception is raised if a feature is requested that is not supported with collocation optimization.
%
% Deprecated: This exception is no longer used by the Ice run time

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef CollocationOptimizationException < Ice.LocalException
    methods
        function obj = CollocationOptimizationException(ice_exid, ice_exmsg)
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:CollocationOptimizationException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.CollocationOptimizationException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
        end
        function id = ice_id(~)
            id = '::Ice::CollocationOptimizationException';
        end
    end
end
