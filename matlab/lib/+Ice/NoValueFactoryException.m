% NoValueFactoryException   Summary of NoValueFactoryException
%
% This exception is raised if no suitable value factory was found during
% unmarshaling of a Slice class instance.
%
% NoValueFactoryException Properties:
%   type_ - The Slice type ID of the class instance for which no no factory could be found.
%
% See also ValueFactory, Communicator.getValueFactoryManager, ValueFactoryManager.add, ValueFactoryManager.find

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef NoValueFactoryException < Ice.MarshalException
    properties
        % type_ - The Slice type ID of the class instance for which no
        % no factory could be found.
        type_ char
    end
    methods
        function obj = NoValueFactoryException(ice_exid, ice_exmsg, reason, type_)
            if nargin <= 2
                reason = '';
                type_ = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:NoValueFactoryException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.NoValueFactoryException';
            end
            obj = obj@Ice.MarshalException(ice_exid, ice_exmsg, reason);
            obj.type_ = type_;
        end
        function id = ice_id(~)
            id = '::Ice::NoValueFactoryException';
        end
    end
end
