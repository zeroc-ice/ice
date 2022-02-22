%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

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

        function r = idToClass(id)
            assert(length(id) > 2 && strcmp(id(1:2), '::'));
            ids = strsplit(id, '::');
            ids = ids(2:end); % Skip empty leading element.
            %
            % Escape any elements that match a keyword.
            %
            for i = 1:length(ids)
                if ismember(ids{i}, IceInternal.Util.keywords)
                    ids{i} = strcat(ids{i}, '_');
                end
            end
            r = strjoin(ids, '.');
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
    properties(Constant, Access=private)
        %
        % Keyword list. *Must* be kept in alphabetical order and synchronized with the list in slice2matlab.
        %
        keywords = { 'break', 'case', 'catch', 'classdef', 'continue', 'else', 'elseif', 'end', 'enumeration', ...
                     'events', 'for', 'function', 'global', 'if', 'methods', 'otherwise', 'parfor', 'persistent', ...
                     'properties', 'return', 'spmd', 'switch', 'try', 'while' };
    end
end
