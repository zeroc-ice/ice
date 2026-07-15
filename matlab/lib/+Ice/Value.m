classdef (Abstract) Value < matlab.mixin.Copyable
    %VALUE The base class for instances of Slice-defined classes.
    %
    %   Value Methods:
    %     Value - Constructs a new Value instance.
    %     ice_getSlicedData - Gets the sliced data if this value was sliced during unmarshaling.
    %     ice_id - Gets the Slice type ID of the most-derived class supported by this object.
    %     ice_postUnmarshal - Ice invokes this method after unmarshaling the fields of this object.
    %     ice_preMarshal - Ice invokes this method prior to marshaling the fields of this object.
    %     ice_staticId - Returns the Slice type ID associated with this type.

    % Copyright (c) ZeroC, Inc.

    methods
        function obj = Value()
            %VALUE Constructs a new Value instance.
            %
            %   Output Arguments
            %     obj - The new Value instance.

            %
            % We need to assign each Value instance a unique identifier for marshaling purposes. This persistent
            % variable retains its setting as long as there is at least one Value instance still active. Once the
            % last instance is collected, the variable gets cleared and we start over again.
            %
            persistent index;
            if isempty(index)
                index = int32(0);
            end
            index = index + 1;
            %assert(index > 0); % Check for rollover
            obj.iceInternal_ = index;
        end

        function ice_preMarshal(~)
            %ICE_PREMARSHAL Ice invokes this method prior to marshaling the fields of this object. This allows a
            %   subclass to override this method in order to update or validate its fields before marshaling.
        end

        function ice_postUnmarshal(~)
            %ICE_POSTUNMARSHAL Ice invokes this method after unmarshaling the fields of this object. This allows a
            %   subclass to override this method in order to update or validate its fields after unmarshaling.
        end

        function r = ice_getSlicedData(obj)
            %ICE_GETSLICEDDATA Gets the sliced data associated with this instance.
            %
            %   Output Arguments
            %     r - The sliced data if this value was sliced during unmarshaling, or an empty array if the value was
            %       not sliced. Unknown slices are preserved only when the sender uses the sliced format.
            %       Ice.SlicedData scalar | empty array of Ice.SlicedData
            r = obj.iceSlicedData_;
        end
    end
    methods (Abstract)
        %ICE_ID Gets the Slice type ID of the most-derived class supported by this object.
        %
        %   Output Arguments
        %     id - The Slice type ID.
        %       character vector
        id = ice_id(obj)
    end
    methods (Static)
        function id = ice_staticId()
            %ICE_STATICID Returns the Slice type ID associated with this type.
            %
            %   Output Arguments
            %     id - The Slice type ID ('::Ice::Object').
            %       character vector
            id = '::Ice::Object';
        end
    end
    methods (Hidden)
        function iceWrite(obj, os)
            os.startValue(obj.iceSlicedData_);
            obj.iceWriteImpl(os);
            os.endValue();
        end
        function iceRead(obj, is)
            is.startValue();
            obj.iceReadImpl(is);
            obj.iceSlicedData_ = is.endValue();
        end
        function r = iceDelayPostUnmarshal(~)
            %
            % Overridden by subclasses that need to do some post-processing after the initial round of
            % unmarshaling is complete.
            %
            r = false;
        end
        function icePostUnmarshal(~)
            %
            % Overridden by subclasses that need to do some post-processing after the initial round of
            % unmarshaling is complete.
            %
        end
    end
    methods (Access = protected)
        function iceWriteImpl(~, ~)
        end
        function iceReadImpl(~, ~)
        end
    end
    properties (Hidden, NonCopyable)
        iceInternal_ int32
        iceSlicedData_ Ice.SlicedData
    end
end
