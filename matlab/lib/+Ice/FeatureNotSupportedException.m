% FeatureNotSupportedException   Summary of FeatureNotSupportedException
%
% This exception is raised if an unsupported feature is used. The unsupported feature string contains the name of the
% unsupported feature.
%
% FeatureNotSupportedException Properties:
%   unsupportedFeature - The name of the unsupported feature.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef FeatureNotSupportedException < Ice.LocalException
    properties
        % unsupportedFeature - The name of the unsupported feature.
        unsupportedFeature char
    end
    methods
        function obj = FeatureNotSupportedException(errID, msg, unsupportedFeature)
            if nargin <= 2
                unsupportedFeature = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:FeatureNotSupportedException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.FeatureNotSupportedException';
            end
            obj = obj@Ice.LocalException(errID, msg);
            obj.unsupportedFeature = unsupportedFeature;
        end
    end
end
