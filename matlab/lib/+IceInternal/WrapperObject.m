classdef (Hidden, Abstract) WrapperObject < handle
    % Copyright (c) ZeroC, Inc.

    methods
        function obj = WrapperObject(impl, type)
            if nargin > 0 % don't do anything for the default constructor
                %
                % If no type was supplied, we convert the class name into the prefix that we'll use for invoking
                % external C functions.
                %
                if nargin == 1
                    type = strrep(class(obj), '.', '_'); % E.g., Ice.Communicator -> Ice_Communicator
                end
                obj.impl_ = impl;
                obj.type_ = type;
            end
        end
    end
    methods (Hidden)
        function delete(obj)
            if ~isempty(obj.impl_)
                obj.iceCall('unref');
                obj.impl_ = [];
            end
        end
        function iceCall(obj, fn, varargin)
            name = strcat(obj.type_, '_', fn);
            ex = calllib('ice', name, obj.impl_, varargin{:});
            if ~isempty(ex)
                ex.throwAsCaller();
            end
        end
        function r = iceCallWithResult(obj, fn, varargin)
            name = strcat(obj.type_, '_', fn);
            result = calllib('ice', name, obj.impl_, varargin{:});
            if isempty(result)
                r = result;
            elseif ~isempty(result.exception)
                result.exception.throwAsCaller();
            else
                r = result.result;
            end
        end
        function r = iceGetImpl(obj)
            r = obj.impl_;
        end
    end
    properties (Hidden, SetAccess = protected)
        impl_
        type_
    end
end
