%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Util
    methods(Static)
        %
        % Internal method - invoke a C function with the given name.
        % The value for fn MUST be given in single quotes!
        %
        function call(fn, varargin)
            ex = calllib('icematlab', fn, varargin{:});
            if ~isempty(ex)
                ex.throwAsCaller();
            end
        end

        %
        % Internal method - invoke a C function representing a global function.
        % The value for fn MUST be given in single quotes!
        %
        function r = callWithResult(fn, varargin)
            result = calllib('icematlab', fn, varargin{:});
            if isempty(result)
                r = result;
            elseif ~isempty(result.exception)
                result.exception.throwAsCaller();
            else
                r = result.result;
            end
        end

        %
        % Internal method - invoke a C function representing an object method.
        % The value for fn MUST be given in single quotes!
        %
        function callMethod(obj, fn, varargin)
            name = replace(strcat(class(obj), '.', fn), '.', '_');
            ex = calllib('icematlab', name, obj.impl, varargin{:});
            if ~isempty(ex)
                ex.throwAsCaller();
            end
        end

        %
        % Internal method - invoke a C function representing an object method.
        % The value for fn MUST be given in single quotes!
        %
        function r = callMethodWithResult(obj, fn, varargin)
            name = replace(strcat(class(obj), '.', fn), '.', '_');
            result = calllib('icematlab', name, obj.impl, varargin{:});
            if isempty(result)
                r = result;
            elseif ~isempty(result.exception)
                result.exception.throwAsCaller();
            else
                r = result.result;
            end
        end

        %
        % Internal method - invoke a C function representing an object method.
        % The value for fn MUST be given in single quotes!
        %
        function callMethodOnType(obj, type, fn, varargin)
            name = strcat(type, '_', fn);
            ex = calllib('icematlab', name, obj.impl, varargin{:});
            if ~isempty(ex)
                ex.throwAsCaller();
            end
        end

        %
        % Internal method - invoke a C function representing an object method.
        % The value for fn MUST be given in single quotes!
        %
        function r = callMethodOnTypeWithResult(obj, type, fn, varargin)
            name = strcat(type, '_', fn);
            result = calllib('icematlab', name, obj.impl, varargin{:});
            if isempty(result)
                r = result;
            elseif ~isempty(result.exception)
                result.exception.throwAsCaller();
            else
                r = result.result;
            end
        end

        function r = idToClass(id)
            assert(startsWith(id, '::'));
            r = extractAfter(replace(id, '::', '.'), 1);
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
