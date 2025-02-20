% DispatchException   Summary of DispatchException
%
% The dispatch failed. This is the base class for local exceptions that can be marshaled and transmitted "over the
% wire".
%
% DispatchException Properties:
%   replyStatus - The reply status byte.

% Copyright (c) ZeroC, Inc.

classdef DispatchException < Ice.LocalException
    properties
        % replyStatus - The reply status byte.
        replyStatus uint8
    end
    methods
        function obj = DispatchException(replyStatus, errID, what)
            if nargin == 0 % default constructor
                replyStatus = Ice.ReplyStatus.UnknownException;
                superArgs = {};
            else
                assert(nargin == 3, 'Invalid number of arguments');
                superArgs = {errID, what};
            end

            obj@Ice.LocalException(superArgs{:});
            obj.replyStatus = replyStatus;
        end
    end
end
