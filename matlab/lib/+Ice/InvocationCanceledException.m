% InvocationCanceledException   Summary of InvocationCanceledException
%
% This exception indicates that an asynchronous invocation failed because it was canceled explicitly by the user.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef InvocationCanceledException < Ice.LocalException
    methods
        function obj = InvocationCanceledException(errID, msg)
            if nargin == 0 || isempty(errID)
                errID = 'Ice:InvocationCanceledException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.InvocationCanceledException';
            end
            obj = obj@Ice.LocalException(errID, msg);
        end
    end
end
