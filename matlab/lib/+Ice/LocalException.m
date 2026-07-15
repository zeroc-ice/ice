classdef LocalException < Ice.Exception
    %LOCALEXCEPTION Base class for all Ice exceptions not defined in Slice.
    %   The identifier of an Ice.LocalException is 'Ice:<Name>Exception' (for example 'Ice:TimeoutException'). Ice
    %   local exceptions that do not have a corresponding MATLAB class are reported as instances of this class; their
    %   identifier carries the underlying exception name.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden)
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
