% FeatureNotSupportedException   Summary of FeatureNotSupportedException
%
% This exception is raised if an unsupported feature is used. The
% unsupported feature string contains the name of the unsupported
% feature
%
% FeatureNotSupportedException Properties:
%   unsupportedFeature - The name of the unsupported feature.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef FeatureNotSupportedException < Ice.LocalException
    properties
        % unsupportedFeature - The name of the unsupported feature.
        unsupportedFeature char
    end
    methods
        function obj = FeatureNotSupportedException(ice_exid, ice_exmsg, unsupportedFeature)
            if nargin <= 2
                unsupportedFeature = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:FeatureNotSupportedException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.FeatureNotSupportedException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.unsupportedFeature = unsupportedFeature;
        end
        function id = ice_id(~)
            id = '::Ice::FeatureNotSupportedException';
        end
    end
end
