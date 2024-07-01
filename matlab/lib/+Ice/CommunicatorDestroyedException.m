% CommunicatorDestroyedException   Summary of CommunicatorDestroyedException
%
% This exception is raised if the Communicator has been destroyed.
%
% See also Communicator.destroy

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef CommunicatorDestroyedException < Ice.LocalException
    methods
        function obj = CommunicatorDestroyedException(errID, msg)
            if nargin == 0 || isempty(errID)
                errID = 'Ice:CommunicatorDestroyedException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.CommunicatorDestroyedException';
            end
            obj = obj@Ice.LocalException(errID, msg);
        end
    end
end
