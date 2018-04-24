classdef (Abstract) UserException < Ice.Exception
    % UserException   Summary of UserException
    %
    % Base class for Slice user exceptions.
    %
    % UserException Methods:
    %   ice_getSlicedData - Obtain the SlicedData object that contains the
    %     marshaled state of any slices for unknown exception types.

    % Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

    methods
        function obj = UserException(id, msg)
            obj = obj@Ice.Exception(id, msg)
        end
        % ice_getSlicedData - Obtain the SlicedData object that contains the
        %   marshaled state of any slices for unknown exception types.
        %
        % Returns (Ice.SlicedData) - The marshaled state of any slices for
        %   unknown exception types.

        function r = ice_getSlicedData(obj)
            r = [];
        end
    end
    methods(Hidden=true)
        function obj = iceRead(obj, is)
            is.startException();
            obj = obj.iceReadImpl(is);
            is.endException(false);
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
