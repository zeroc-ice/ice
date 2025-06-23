classdef DispatchException < Ice.LocalException
    %DISPATCHEXCEPTION The dispatch failed. This is the base class for local exceptions that can be marshaled and
    %   transmitted "over the wire".
    %
    %   DispatchException Properties:
    %     replyStatus - The reply status byte.

    % Copyright (c) ZeroC, Inc.

    properties
        %REPLYSTATUS The reply status byte.
        %   uint8 scalar
        replyStatus (1, 1) uint8 = Ice.ReplyStatus.UnknownException
    end
    methods(Hidden)
        function obj = DispatchException(replyStatus, errID, what)
            if nargin == 0 % default constructor
                superArgs = {};
            else
                assert(nargin == 3, 'Invalid number of arguments');
                superArgs = {errID, what};
            end
            obj@Ice.LocalException(superArgs{:});
            if nargin > 0
                obj.replyStatus = replyStatus;
            end
        end
    end
end
