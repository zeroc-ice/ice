% UnknownException   Summary of UnknownException
%
% This exception is raised if an operation call on a server raises an
% unknown exception. For example, for C++, this exception is raised
% if the server throws a C++ exception that is not directly or
% indirectly derived from Ice::LocalException or
% Ice::UserException.
%
% UnknownException Properties:
%   unknown - This field is set to the textual representation of the unknown exception if available.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef UnknownException < Ice.LocalException
    properties
        % unknown - This field is set to the textual representation of the unknown
        % exception if available.
        unknown char
    end
    methods
        function obj = UnknownException(ice_exid, ice_exmsg, unknown)
            if nargin <= 2
                unknown = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:UnknownException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.UnknownException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.unknown = unknown;
        end
        function id = ice_id(~)
            id = '::Ice::UnknownException';
        end
    end
end
