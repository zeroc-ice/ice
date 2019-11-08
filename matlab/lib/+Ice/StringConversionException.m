% StringConversionException   Summary of StringConversionException
%
% This exception is raised when a string conversion to or from UTF-8
% fails during marshaling or unmarshaling.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef StringConversionException < Ice.MarshalException
    methods
        function obj = StringConversionException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:StringConversionException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.StringConversionException';
            end
            obj = obj@Ice.MarshalException(ice_exid, ice_exmsg, reason);
        end
        function id = ice_id(~)
            id = '::Ice::StringConversionException';
        end
    end
end
