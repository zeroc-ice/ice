classdef (Abstract) UserException < Ice.Exception
    %USEREXCEPTION
    %   Abstract base class for all Ice exceptions defined in Slice.

    % Copyright (c) ZeroC, Inc.

    methods(Abstract, Hidden)
        ice_id(obj)
    end
    methods(Hidden)
        function obj = iceRead(obj, is)
            is.startException();
            obj = obj.iceReadImpl(is);
            is.endException();
        end
        function obj = icePostUnmarshal(obj)
            %
            % Overridden by subclasses that have class members.
            %
        end
    end
    methods(Abstract,Access=protected)
        obj = iceReadImpl(obj, is)
    end
end
