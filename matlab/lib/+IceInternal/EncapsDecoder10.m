%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

classdef EncapsDecoder10 < IceInternal.EncapsDecoder
    methods
        function obj = EncapsDecoder10(is, encaps, sliceValues, valueFactoryManager, classResolver)
            obj = obj@IceInternal.EncapsDecoder(is, encaps, sliceValues, valueFactoryManager, classResolver);
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

            if ~isempty(obj.patchMap) && obj.patchMap.Count > 0
                %
                % If any entries remain in the patch map, the sender has sent an index for an object, but failed
                % to supply the object.
                %
                throw(Ice.MarshalException('', '', 'index for class received, but no instance'));
            end
        end

        function readValue(obj, cb)
            assert(~isempty(cb));

            %
            % Object references are encoded as a negative integer in 1.0.
            %
            index = obj.is.readInt();
            if index > 0
                throw(Ice.MarshalException('', '', 'invalid object id'));
            end
            index = -index;

            if index == 0
                cb([]);
            else
                obj.addPatchEntry(index, cb);
            end
        end

        function throwException(obj)
            assert(obj.sliceType == IceInternal.SliceType.NoSlice);

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
                %
                % Use the class resolver to convert the type ID into a constructor.
                %
                constructor = obj.classResolver.resolve(obj.typeId);

                %
                % Try to instantiate the class.
                %
                ex = [];
                if ~isempty(constructor)
                    try
                        ex = constructor(); % Invoke the constructor.
                    catch e
                        %
                        % Instantiation failed.
                        %
                        reason = sprintf('exception in constructor for %s', obj.typeId);
                        me = Ice.MarshalException('', reason, reason);
                        me.addCause(e);
                        throw(me);
                    end
                end

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
                        if isa(ex, 'Ice.UnmarshalOutOfBoundsException')
                            %
                            % An oversight in the 1.0 encoding means there is no marker to indicate
                            % the last slice of an exception. As a result, we just try to read the
                            % next type ID, which raises UnmarshalOutOfBoundsException when the
                            % input buffer underflows.
                            %
                            % Set the reason member to a more helpful message.
                            %
                            ex.reason = ['unknown exception type `', mostDerivedId, ''''];
                        end
                        rethrow(ex);
                    end
                end
            end
        end

        function startInstance(obj, sliceType)
            assert(obj.sliceType == sliceType);
            obj.skipFirstSlice = true;
        end

        function r = endInstance(obj, preserve)
            %
            % Read the Ice::Value slice.
            %
            if obj.sliceType == IceInternal.SliceType.ValueSlice
                obj.startSlice();
                sz = obj.is.readSize(); % For compatibility with the old AFM.
                if sz ~= 0
                    throw(Ice.MarshalException('', '', 'invalid Object slice'));
                end
                obj.endSlice();
            end
            obj.sliceType = IceInternal.SliceType.NoSlice;
            r = [];
        end

        function r = startSlice(obj)
            %
            % If first slice, don't read the header, it was already read in
            % readInstance or throwException to find the type.
            %
            if obj.skipFirstSlice
                obj.skipFirstSlice = false;
                r = obj.typeId;
                return;
            end

            %
            % For values, first read the type ID boolean which indicates
            % whether or not the type ID is encoded as a string or as an
            % index. For exceptions, the type ID is always encoded as a
            % string.
            %
            if obj.sliceType == IceInternal.SliceType.ValueSlice
                isIndex = obj.is.readBool();
                obj.typeId = obj.readTypeId(isIndex);
            else
                obj.typeId = obj.is.readString();
            end

            obj.sliceSize = obj.is.readInt();
            if obj.sliceSize < 4
                throw(Ice.UnmarshalOutOfBoundsException());
            end

            r = obj.typeId;
        end

        function endSlice(obj)
            % Nothing to do
        end

        function skipSlice(obj)
            %obj.is.traceSkipSlice(obj.typeId, obj.sliceType);
            assert(obj.sliceSize >= 4);
            obj.is.skip(obj.sliceSize - 4);
        end

        function r = readOptional(obj, readTag, expectedFormat)
            r = false;
        end
    end
    methods(Access=private)
        function readInstance(obj)
            index = obj.is.readInt();

            if index <= 0
                throw(Ice.MarshalException('', '', 'invalid object id'));
            end

            obj.sliceType = IceInternal.SliceType.ValueSlice;
            obj.skipFirstSlice = false;

            %
            % Read the first slice header.
            %
            obj.startSlice();
            mostDerivedId = obj.typeId;
            v = [];
            while true
                %
                % For the 1.0 encoding, the type ID for the base Object class
                % marks the last slice.
                %
                if strcmp(obj.typeId, Ice.Value.ice_staticId())
                    throw(Ice.NoValueFactoryException('', '', '', mostDerivedId));
                end

                v = obj.newInstance(obj.typeId);

                %
                % We found a factory, we get out of this loop.
                %
                if ~isempty(v)
                    break;
                end

                %
                % If slicing is disabled, stop unmarshaling.
                %
                if ~obj.sliceValues
                    reason = 'no value factory found and slicing is disabled';
                    throw(Ice.NoValueFactoryException('', reason, reason, obj.typeId));
                end

                %
                % Slice off what we don't understand.
                %
                obj.skipSlice();
                obj.startSlice(); % Read next Slice header for next iteration.
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
    end
end
