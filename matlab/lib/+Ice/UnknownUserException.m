% UnknownUserException   Summary of UnknownUserException
%
% An operation raised an incorrect user exception. This exception is raised if an operation raises a user exception
% that is not declared in the exception's throws clause. Such undeclared exceptions are not transmitted
% from the server to the client by the Ice protocol, but instead the client just gets an UnknownUserException.
% This is necessary in order to not violate the contract established by an operation's signature: Only local
% exceptions and user exceptions declared in the throws clause can be raised.

%  Copyright (c) ZeroC, Inc.

classdef (Sealed) UnknownUserException < Ice.UnknownException
    methods
        function obj = UnknownUserException(typeID, what)
            errID = 'Ice:UnknownUserException';
            if nargin == 0
                superArgs = {};
            elseif nargin == 1
                superArgs = {errID,...
                    sprintf('The invocation returned an exception that does not conform to the operation''s exception specification: %s', typeID)};
            else
                assert(nargin == 2, 'Invalid number of arguments');
                superArgs = {errID, what}; % we ignore typeID in this case
            end
            obj@Ice.UnknownException(superArgs{:});
        end
    end
end
