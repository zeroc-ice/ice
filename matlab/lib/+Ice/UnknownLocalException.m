% UnknownLocalException   Summary of UnknownLocalException
%
% This exception is raised if an operation call on a server raises a
% local exception. Because local exceptions are not transmitted by
% the Ice protocol, the client receives all local exceptions raised
% by the server as UnknownLocalException. The only exception to this
% rule are all exceptions derived from RequestFailedException,
% which are transmitted by the Ice protocol even though they are
% declared local.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef UnknownLocalException < Ice.UnknownException
    methods
        function obj = UnknownLocalException(ice_exid, ice_exmsg, unknown)
            if nargin <= 2
                unknown = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:UnknownLocalException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.UnknownLocalException';
            end
            obj = obj@Ice.UnknownException(ice_exid, ice_exmsg, unknown);
        end
        function id = ice_id(~)
            id = '::Ice::UnknownLocalException';
        end
    end
end
