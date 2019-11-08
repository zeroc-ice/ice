% CloneNotImplementedException   Summary of CloneNotImplementedException
%
% An attempt was made to clone a class that does not support
% cloning.
%
% This exception is raised if ice_clone is called on
% a class that is derived from an abstract Slice class (that is,
% a class containing operations), and the derived class does not
% provide an implementation of the ice_clone operation (C++ only).

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef CloneNotImplementedException < Ice.LocalException
    methods
        function obj = CloneNotImplementedException(ice_exid, ice_exmsg)
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:CloneNotImplementedException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.CloneNotImplementedException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
        end
        function id = ice_id(~)
            id = '::Ice::CloneNotImplementedException';
        end
    end
end
