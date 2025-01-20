classdef LocalException < Ice.Exception
    % LocalException   Summary of LocalException
    %
    % Base class for all Ice exceptions not defined in Slice.

    % Copyright (c) ZeroC, Inc.

    methods
        function obj = LocalException(errID, what)
            if nargin == 0 % default constructor
                superArgs = {};
            else
                assert(nargin == 2, 'Invalid number of arguments');
                % Since the what message (usually from C++) can contain random characters, we can't use it directly as
                % the msg.
                superArgs = {errID, '%s', what};
            end
            obj@Ice.Exception(superArgs{:});
        end
    end
end
