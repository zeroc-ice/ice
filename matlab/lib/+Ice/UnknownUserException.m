% UnknownUserException   Summary of UnknownUserException
%
% An operation raised an incorrect user exception. This exception is raised if an operation raises a user exception
% that is not declared in the exception's throws clause. Such undeclared exceptions are not transmitted
% from the server to the client by the Ice protocol, but instead the client just gets an UnknownUserException.
% This is necessary in order to not violate the contract established by an operation's signature: Only local
% exceptions and user exceptions declared in the throws clause can be raised.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef UnknownUserException < Ice.UnknownException
    methods
        function obj = UnknownUserException(errID, msg, unknown)
            if nargin <= 2
                unknown = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:UnknownUserException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.UnknownUserException';
            end
            obj = obj@Ice.UnknownException(errID, msg, unknown);
        end
    end
end
