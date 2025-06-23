classdef (Abstract) UserException < Ice.Exception
    %USEREXCEPTION Abstract base class for all Ice exceptions defined in Slice.
    %
    %   UserException Methods:
    %     ice_id - Returns the Slice type ID associated with this instance.

    % Copyright (c) ZeroC, Inc.

    methods(Abstract)
        %ICE_ID Returns the Slice type ID associated with this instance.
        %
        %   Output Arguments
        %     id - The Slice type ID.
        %       character vector
        id = ice_id(obj)
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
