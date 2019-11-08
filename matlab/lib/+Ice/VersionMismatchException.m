% VersionMismatchException   Summary of VersionMismatchException
%
% This exception is raised if the Ice library version does not match
% the version in the Ice header files.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef VersionMismatchException < Ice.LocalException
    methods
        function obj = VersionMismatchException(ice_exid, ice_exmsg)
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:VersionMismatchException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.VersionMismatchException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
        end
        function id = ice_id(~)
            id = '::Ice::VersionMismatchException';
        end
    end
end
