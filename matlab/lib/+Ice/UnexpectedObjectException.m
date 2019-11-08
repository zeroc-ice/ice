% UnexpectedObjectException   Summary of UnexpectedObjectException
%
% This exception is raised if the type of an unmarshaled Slice class instance does
% not match its expected type.
% This can happen if client and server are compiled with mismatched Slice
% definitions or if a class of the wrong type is passed as a parameter
% or return value using dynamic invocation. This exception can also be
% raised if IceStorm is used to send Slice class instances and
% an operation is subscribed to the wrong topic.
%
% UnexpectedObjectException Properties:
%   type_ - The Slice type ID of the class instance that was unmarshaled.
%   expectedType - The Slice type ID that was expected by the receiving operation.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef UnexpectedObjectException < Ice.MarshalException
    properties
        % type_ - The Slice type ID of the class instance that was unmarshaled.
        type_ char
        % expectedType - The Slice type ID that was expected by the receiving operation.
        expectedType char
    end
    methods
        function obj = UnexpectedObjectException(ice_exid, ice_exmsg, reason, type_, expectedType)
            if nargin <= 2
                reason = '';
                type_ = '';
                expectedType = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:UnexpectedObjectException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.UnexpectedObjectException';
            end
            obj = obj@Ice.MarshalException(ice_exid, ice_exmsg, reason);
            obj.type_ = type_;
            obj.expectedType = expectedType;
        end
        function id = ice_id(~)
            id = '::Ice::UnexpectedObjectException';
        end
    end
end
