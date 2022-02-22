%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

%
% Singleton used to indicate an unset optional value.
%
classdef UnsetI < handle
    methods(Access=private)
        function obj = UnsetI()
        end
    end
    methods
        function r = eq(obj, other)
            r = isequal(obj, other);
        end
        function r = ne(obj, other)
            r = ~isequal(obj, other);
        end
    end
    properties(Constant)
        Instance = IceInternal.UnsetI()
    end
end
