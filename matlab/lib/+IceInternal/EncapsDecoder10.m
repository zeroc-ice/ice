% Copyright (c) ZeroC, Inc.

classdef EncapsDecoder10 < IceInternal.EncapsDecoder
    methods
        function obj = EncapsDecoder10(is, encaps, classGraphDepthMax)
            obj@IceInternal.EncapsDecoder(is, encaps, classGraphDepthMax);
            obj.sliceType = IceInternal.SliceType.NoSlice;
        end

        function readPendingValues(obj)
            num = obj.is.readSize();
            while num > 0
                for k = 1:num
                    obj.readInstance();
                end
                num = obj.is.readSize();
            end

            if obj.patchMapLength > 0
                %
                % If any entries remain in the patch map, the sender has sent an index for an object, but failed
                % to supply the object.
                %
                throw(Ice.MarshalException('index for class received, but no instance'));
            end
        end

        function readValue(obj, cb)
            %
            % Object references are encoded as a negative integer in 1.0.
            %
            index = obj.is.readInt();
            if index > 0
                throw(Ice.MarshalException('invalid object id'));
            end
            index = -index;

            if index == 0
                cb([]);
            else
                obj.addPatchEntry(index, cb);
            end
        end

        function throwException(obj)
            %assert(obj.sliceType == IceInternal.SliceType.NoSlice);

            %
            % User exception with the 1.0 encoding start with a boolean flag
            % that indicates whether or not the exception has classes.
            %
            % This allows reading the pending values even if some part of
            % the exception was sliced.
            %
            usesClasses = obj.is.readBool();

            obj.sliceType = IceInternal.SliceType.ExceptionSlice;
            obj.skipFirstSlice = false;

            %
            % Read the first slice header.
            %
            obj.startSlice();
            mostDerivedId = obj.typeId;
            while true
                ex = obj.is.getCommunicator().getSliceLoader().newInstance(obj.typeId);

                if ~isempty(ex)
                    %
                    % Exceptions are value types so we have to replace 'ex' with its new value after calling methods.
                    %
                    ex = ex.iceRead(obj.is);
                    if usesClasses
                        obj.readPendingValues();
                    end
                    ex = ex.icePostUnmarshal();
                    throw(ex);
                else
                    %
                    % Slice off what we don't understand.
                    %
                    obj.skipSlice();
                    try
                        obj.startSlice();
                    catch ex
                        if isa(ex, 'Ice.MarshalException')
                            %
                            % An oversight in the 1.0 encoding means there is no marker to indicate
                            % the last slice of an exception. As a result, we just try to read the
                            % next type ID, which raises MarshalException when the input buffer underflows.
                            %
                            throw(Ice.MarshalException(sprintf('unknown exception type ''%s''', mostDerivedId)));
                        end
                        rethrow(ex);
                    end
                end
            end
        end

        function startInstance(obj, ~)
            %assert(obj.sliceType == sliceType);
            obj.skipFirstSlice = true;
        end

        function r = endInstance(obj, ~)
            %
            % Read the Ice::Value slice.
            %
            if obj.sliceType == IceInternal.SliceType.ValueSlice
                obj.startSlice();
                sz = obj.is.readSize(); % For compatibility with the old AFM.
                if sz ~= 0
                    throw(Ice.MarshalException('invalid Object slice'));
                end
                obj.endSlice();
            end
            obj.sliceType = IceInternal.SliceType.NoSlice;
            r = [];
        end

        function startSlice(obj)
            %
            % If first slice, don't read the header, it was already read in
            % readInstance or throwException to find the type.
            %
            if obj.skipFirstSlice
                obj.skipFirstSlice = false;
                return;
            end

            %
            % For values, first read the type ID boolean which indicates
            % whether or not the type ID is encoded as a string or as an
            % index. For exceptions, the type ID is always encoded as a
            % string.
            %
            if obj.sliceType == IceInternal.SliceType.ValueSlice
                if obj.is.readBool()
                    typeIdIndex = obj.is.readSize();
                    obj.typeIdIndex = typeIdIndex;
                    obj.typeId = obj.getTypeId(obj.typeIdIndex);
                else
                    [obj.typeIdIndex, obj.typeId] = obj.readTypeId();
                end
            else
                obj.typeId = obj.is.readString();
            end

            obj.sliceSize = obj.is.readInt();
            if obj.sliceSize < 4
                throw(Ice.MarshalException('invalid slice size'));
            end
        end

        function endSlice(~)
            % Nothing to do
        end

        function skipSlice(obj)
            %obj.is.traceSkipSlice(obj.typeId, obj.sliceType);
            %assert(obj.sliceSize >= 4);
            obj.is.skip(obj.sliceSize - 4);
        end

    end
    methods(Access=private)
        function readInstance(obj)
            index = obj.is.readInt();
            if index <= 0
                throw(Ice.MarshalException('invalid object id'));
            end

            obj.sliceType = IceInternal.SliceType.ValueSlice;
            obj.skipFirstSlice = false;

            %
            % Read the first slice header.
            %
            obj.startSlice();
            mostDerivedId = obj.typeId;
            while true
                %
                % For the 1.0 encoding, the type ID for the base Object class
                % marks the last slice.
                %
                if strcmp(obj.typeId, Ice.Value.ice_staticId())
                    throw(Ice.MarshalException(sprintf('The Slice loader did not find a class for type ID ''%s''.', mostDerivedId)));
                end

                v = obj.newInstance(obj.typeId);

                %
                % We found a factory, we get out of this loop.
                %
                if ~isempty(v)
                    break;
                end

                %
                % Slice off what we don't understand.
                %
                obj.skipSlice();
                obj.startSlice(); % Read next Slice header for next iteration.
            end

            %
            % Compute the biggest class graph depth of this object. To compute this,
            % we get the class graph depth of each ancestor from the patch map and
            % keep the biggest one.
            %
            obj.classGraphDepth = 0;

            if index <= length(obj.patchMap)
                pl = obj.patchMap{index};
                for i = 1:length(pl)
                    entry = pl{i};
                    classGraphDepth = entry.classGraphDepth;
                    if classGraphDepth > obj.classGraphDepth
                        obj.classGraphDepth = classGraphDepth;
                    end
                end
            end

            obj.classGraphDepth = obj.classGraphDepth + 1;
            if obj.classGraphDepth > obj.classGraphDepthMax
                throw(Ice.MarshalException('maximum class graph depth reached'));
            end

            %
            % Unmarshal the instance and add it to the map of unmarshaled instances.
            %
            obj.unmarshal(index, v);
        end
    end
    properties(Access=private)
        sliceType
        skipFirstSlice
        sliceSize
        typeId
        typeIdIndex
    end
end
