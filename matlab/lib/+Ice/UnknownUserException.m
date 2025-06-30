classdef (Sealed) UnknownUserException < Ice.UnknownException
    %UNKNOWNUSEREXCEPTION
    %   The exception that is thrown when a client receives a UserException that was not declared in the operation's
    %   exception specification.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden)
        function obj = UnknownUserException(typeID, what)
            errID = 'Ice:UnknownUserException';
            if nargin == 0
                superArgs = {};
            elseif nargin == 1
                superArgs = {uint8(Ice.ReplyStatus.UnknownUserException), errID,...
                    sprintf('The invocation returned an exception that does not conform to the operation''s exception specification: %s', typeID)};
            else
                assert(nargin == 2, 'Invalid number of arguments');
                superArgs = {uint8(Ice.ReplyStatus.UnknownUserException), errID, what}; % we ignore typeID in this case
            end
            obj@Ice.UnknownException(superArgs{:});
        end
    end
end
