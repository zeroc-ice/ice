% Copyright (c) ZeroC, Inc.

classdef Util
    methods(Static)
        %
        % Internal method - invoke a C function with the given name.
        % The value for fn MUST be given in single quotes!
        %
        function call(fn, varargin)
            ex = calllib('ice', fn, varargin{:});
            if ~isempty(ex)
                ex.throwAsCaller();
            end
        end

        %
        % Internal method - invoke a C function representing a global function.
        % The value for fn MUST be given in single quotes!
        %
        function r = callWithResult(fn, varargin)
            result = calllib('ice', fn, varargin{:});
            if isempty(result)
                r = result;
            elseif ~isempty(result.exception)
                result.exception.throwAsCaller();
            else
                r = result.result;
            end
        end

        function r = strcmp(s1, s2)
            n = min(length(s1), length(s2));
            for i = 1:n
                if s1(i) < s2(i)
                    r = -1;
                    return;
                elseif s1(i) > s2(i)
                    r = 1;
                    return;
                end
            end
            if length(s1) > n
                r = 1;
            elseif length(s2) > n
                r = -1;
            else
                r = 0;
            end
        end
    end
end
