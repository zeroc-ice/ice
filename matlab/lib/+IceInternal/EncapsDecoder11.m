%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef EncapsDecoder11 < IceInternal.EncapsDecoder
    methods
        function obj = EncapsDecoder11(is, encaps, sliceValues, valueFactoryManager, classResolver, compactIdResolver)
            obj = obj@IceInternal.EncapsDecoder(is, encaps, sliceValues, valueFactoryManager, classResolver);
            obj.compactIdResolver = compactIdResolver;
            obj.current = [];
            obj.valueIdIndex = 1;
        end

        function readValue(obj, cb)
            import IceInternal.Protocol;
            index = obj.is.readSize();
            if index < 0
                throw(Ice.MarshalException('', '', 'invalid object id'));
            elseif index == 0
                if ~isempty(cb)
                    cb([]);
                end
            elseif ~isempty(obj.current) && bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_INDIRECTION_TABLE)
                %
                % When reading a class instance within a slice and there's an
                % indirect instance table, always read an indirect reference
                % that points to an instance from the indirect instance table
                % marshaled at the end of the Slice.
                %
                % Maintain a list of indirect references. Note that the
                % indirect index starts at 1, so we decrement it by one to
                % derive an index into the indirection table that we'll read
                % at the end of the slice.
                %
                if ~isempty(cb)
                    if isempty(obj.current.indirectPatchList) % Lazy initialization
                        obj.current.indirectPatchList = containers.Map('KeyType', 'int32', 'ValueType', 'any');
                    end
                    e = IceInternal.IndirectPatchEntry();
                    %e.index = index - 1;
                    e.index = index; % MATLAB indexing starts at 1
                    e.cb = cb;
                    sz = length(obj.current.indirectPatchList);
                    obj.current.indirectPatchList(sz) = e;
                end
            else
                obj.readInstance(index, cb);
            end
        end

        function throwException(obj)
            import IceInternal.Protocol;
            assert(isempty(obj.current));

            obj.push(IceInternal.SliceType.ExceptionSlice);

            %
            % Read the first slice header.
            %
            obj.startSlice();
            mostDerivedId = obj.current.typeId;
            while true
                %
                % Use the class resolver to convert the type ID into a class name.
                %
                cls = obj.classResolver.resolve(obj.current.typeId);

                %
                % Try to instantiate the class.
                %
                ex = [];
                if ~isempty(cls)
                    try
                        constructor = str2func(cls); % Get the constructor.
                        ex = constructor(); % Invoke the constructor.
                    catch e
                        %
                        % Instantiation failed.
                        %
                    end
                end

                if ~isempty(ex)
                    ex = ex.read_(obj.is);
                    throw(ex);
                else
                    %
                    % Slice off what we don't understand.
                    %
                    obj.skipSlice();

                    %
                    % If this is the last slice, raise an exception and stop unmarshaling.
                    %
                    if bitand(obj.current.sliceFlags, Protocol.FLAG_IS_LAST_SLICE)
                        throw(Ice.UnknownUserException('', '', mostDerivedId));
                    end

                    obj.startSlice();
                end
            end
        end

        function startInstance(obj, sliceType)
            assert(obj.current.sliceType == sliceType);
            obj.current.skipFirstSlice = true;
        end

        function r = endInstance(obj, preserve)
            slicedData = [];
            if preserve
                slicedData = obj.readSlicedData();
            end
            obj.current.slices = {};
            obj.current.indirectionTables = {};
            obj.current.indirectPatchList = [];
            obj.current = obj.current.previous;
            r = slicedData;
        end

        function r = startSlice(obj)
            import IceInternal.Protocol;
            %
            % If first slice, don't read the header, it was already read in
            % readInstance or throwException to find the factory.
            %
            if obj.current.skipFirstSlice
                obj.current.skipFirstSlice = false;
                r = obj.current.typeId;
                return;
            end

            obj.current.sliceFlags = obj.is.readByte();

            %
            % Read the type ID, for value slices the type ID is encoded as a
            % string or as an index, for exceptions it's always encoded as a
            % string.
            %
            if obj.current.sliceType == IceInternal.SliceType.ValueSlice
                % Must be checked first!
                if bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_TYPE_ID_COMPACT) == Protocol.FLAG_HAS_TYPE_ID_COMPACT
                    obj.current.typeId = '';
                    obj.current.compactId = obj.is.readSize();
                elseif bitand(obj.current.sliceFlags, bitor(Protocol.FLAG_HAS_TYPE_ID_STRING, Protocol.FLAG_HAS_TYPE_ID_INDEX))
                    obj.current.typeId = obj.readTypeId(bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_TYPE_ID_INDEX) > 0);
                    obj.current.compactId = -1;
                else
                    % Only the most derived slice encodes the type ID for the compact format.
                    obj.current.typeId = '';
                    obj.current.compactId = -1;
                end
            else
                obj.current.typeId = obj.is.readString();
            end

            %
            % Read the slice size if necessary.
            %
            if bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_SLICE_SIZE)
                obj.current.sliceSize = obj.is.readInt();
                if obj.current.sliceSize < 4
                    throw(Ice.UnmarshalOutOfBoundsException());
                end
            else
                obj.current.sliceSize = 0;
            end

            r = obj.current.typeId;
        end

        function endSlice(obj)
            import IceInternal.Protocol;
            if bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_OPTIONAL_MEMBERS)
                obj.is.skipOptionals();
            end

            %
            % Read the indirection table if one is present and transform the
            % indirect patch list into patch entries with direct references.
            %
            if bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_INDIRECTION_TABLE)
                %
                % The table is written as a sequence<size> to conserve space.
                %
                sz = obj.is.readAndCheckSeqSize(1);
                indirectionTable = cell(1, sz);
                for i = 1:sz
                    indirectionTable{i} = obj.readInstance(obj.is.readSize(), []);
                end

                %
                % Sanity checks. If there are optional members, it's possible
                % that not all instance references were read if they are from
                % unknown optional data members.
                %
                if length(indirectionTable) == 0
                    throw(Ice.MarshalException('', '', 'empty indirection table'));
                end
                if isempty(obj.current.indirectPatchList) && ...
                   bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_OPTIONAL_MEMBERS) == 0
                    throw(Ice.MarshalException('', '', 'no references to indirection table'));
                end

                %
                % Convert indirect references into direct references.
                %
                if ~isempty(obj.current.indirectPatchList)
                    keys = obj.current.indirectPatchList.keys();
                    for i = 1:length(keys)
                        e = obj.current.indirectPatchList(keys{i});
                        assert(e.index > 0); % MATLAB starts indexing at 1
                        if e.index > length(indirectionTable)
                            throw(Ice.MarshalException('', '', 'indirection out of range'));
                        end
                        obj.addPatchEntry(indirectionTable{e.index}, e.cb);
                    end
                    obj.current.indirectPatchList = [];
                end
            end
        end

        function skipSlice(obj)
            import IceInternal.Protocol;
            %obj.is.traceSkipSlice(obj.current.typeId, obj.current.sliceType); TODO

            start = obj.is.pos();

            if bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_SLICE_SIZE)
                assert(obj.current.sliceSize >= 4);
                obj.is.skip(obj.current.sliceSize - 4);
            else
                if obj.current.sliceType == IceInternal.SliceType.ValueSlice
                    reason = ['no value factory found and compact format prevents ', ...
                              'slicing (the sender should use the sliced format instead)'];
                    throw(Ice.NoValueFactoryException('', reason, reason, obj.current.typeId));
                else
                    throw(Ice.UnknownUserException('', '', obj.current.typeId));
                end
            end

            %
            % Preserve this slice.
            %
            info = Ice.SliceInfo();
            info.typeId = obj.current.typeId;
            info.compactId = obj.current.compactId;
            info.hasOptionalMembers = bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_OPTIONAL_MEMBERS) > 0;
            info.isLastSlice = bitand(obj.current.sliceFlags, Protocol.FLAG_IS_LAST_SLICE) > 0;
            if info.hasOptionalMembers
                %
                % Don't include the optional member end marker. It will be re-written by
                % endSlice when the sliced data is re-written.
                %
                info.bytes = obj.is.getBytes(start, obj.is.pos() - 1);
            else
                info.bytes = obj.is.getBytes(start, obj.is.pos());
            end

            %
            % Read the indirect instance table. We read the instances or their
            % IDs if the instance is a reference to an already unmarshaled
            % object.
            %
            % The SliceInfo object sequence is initialized only if
            % readSlicedData is called.
            %
            if bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_INDIRECTION_TABLE)
                sz = obj.is.readAndCheckSeqSize(1);
                indirectionTable = cell(1, sz);
                for i = 1:sz
                    indirectionTable{i} = obj.readInstance(obj.is.readSize(), []);
                end
                obj.current.indirectionTables{end + 1} = indirectionTable;
            else
                obj.current.indirectionTables{end + 1} = {};
            end

            obj.current.slices{end + 1} = info;
        end

        function r = readOptional(obj, readTag, expectedFormat)
            import IceInternal.Protocol;
            if isempty(obj.current)
                r = obj.is.readOptionalImpl(readTag, expectedFormat);
                return;
            elseif bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_OPTIONAL_MEMBERS)
                r = obj.is.readOptionalImpl(readTag, expectedFormat);
                return;
            end
            r = false;
        end

    end
    methods(Access=private)
        function r = readInstance(obj, index, cb)
            import IceInternal.Protocol;
            assert(index > 0);

            if index > 1
                if ~isempty(cb)
                    obj.addPatchEntry(index, cb);
                end
                r = index;
                return;
            end

            obj.push(IceInternal.SliceType.ValueSlice);

            %
            % Get the instance ID before we start reading slices. If some
            % slices are skipped, the indirect instance table is still read and
            % might read other instances.
            %
            obj.valueIdIndex = obj.valueIdIndex + 1;
            index = obj.valueIdIndex;

            %
            % Read the first slice header.
            %
            obj.startSlice();
            mostDerivedId = obj.current.typeId;
            v = [];
            while true
                updateCache = false;

                if obj.current.compactId >= 0
                    updateCache = true;

                    %
                    % Translate a compact (numeric) type ID into a class.
                    %
                    if isempty(obj.compactIdCache) % Lazy initialization.
                        obj.compactIdCache = containers.Map('KeyType', 'int32', 'ValueType', 'char');
                    else
                        %
                        % Check the cache to see if we've already translated the compact type ID into a class.
                        %
                        if obj.compactIdCache.isKey(obj.current.compactId)
                            cls = obj.compactIdCache(obj.current.compactId);
                            try
                                constructor = str2func(cls); % Get the constructor.
                                v = constructor(); % Invoke the constructor.
                                updateCache = false;
                            catch e
                                reason = sprintf('constructor failed for class %s with compact id %d', cls, ...
                                                 obj.current.compactId);
                                ex = Ice.NoValueFactoryException('', reason, reason, '');
                                ex.addCause(e);
                                throw(ex);
                            end
                        end
                    end

                    %
                    % If we haven't already cached a class for the compact ID, then try to translate the
                    % compact ID into a type ID.
                    %
                    if isempty(v)
                        obj.current.typeId = '';
                        if ~isempty(obj.compactIdResolver)
                            try
                                obj.current.typeId = obj.compactIdResolver(obj.current.compactId);
                            catch ex
                                if isa(ex, 'Ice.LocalException')
                                    rethrow(ex);
                                else
                                    throw(Ice.MarshalException('', '', ...
                                            sprintf('exception in compact ID resolver for ID %d', ...
                                                    obj.current.compactId)));
                                end
                            end
                        end

                        if isempty(obj.current.typeId)
                            obj.current.typeId = obj.resolveCompactId(obj.current.compactId);
                        end
                    end
                end

                if isempty(v) && ~isempty(obj.current.typeId)
                    v = obj.newInstance(obj.current.typeId);
                end

                if ~isempty(v)
                    if updateCache
                        assert(obj.current.compactId >= 0);
                        obj.compactIdCache(obj.current.compactId) = class(v);
                    end

                    %
                    % We have an instance, get out of this loop.
                    %
                    break;
                end

                %
                % If slicing is disabled, stop unmarshaling.
                %
                if ~obj.sliceValues
                    reason = 'no value factory found and slicing is disabled';
                    throw(Ice.NoValueFactoryException('', reason, reason, obj.current.typeId));
                end

                %
                % Slice off what we don't understand.
                %
                obj.skipSlice();

                %
                % If this is the last slice, keep the instance as an opaque
                % UnknownSlicedValue object.
                %
                if bitand(obj.current.sliceFlags, Protocol.FLAG_IS_LAST_SLICE)
                    %
                    % Provide a factory with an opportunity to supply the instance.
                    % We pass the "::Ice::Object" ID to indicate that this is the
                    % last chance to preserve the instance.
                    %
                    v = obj.newInstance(Ice.Value.ice_staticId());
                    if isempty(v)
                        v = Ice.UnknownSlicedValue(mostDerivedId);
                    end

                    break;
                end

                obj.startSlice(); % Read next Slice header for next iteration.
            end

            %
            % Unmarshal the instance.
            %
            obj.unmarshal(index, v);

            if isempty(obj.current) && ~isempty(obj.patchMap)
                %
                % If any entries remain in the patch map, the sender has sent an index for an instance, but failed
                % to supply the instance.
                %
                throw(Ice.MarshalException('', '', 'index for class received, but no instance'));
            end

            if ~isempty(cb)
                cb(v);
            end

            r = index;
        end

        function r = readSlicedData(obj)
            if isempty(obj.current.slices) % No preserved slices.
                r = [];
            end

            %
            % The _indirectionTables member holds the indirection table for each slice
            % in _slices.
            %
            assert(length(obj.current.slices) == length(obj.current.indirectionTables));
            function setInstance(si, n, v)
                si.instances{n} = v;
            end
            for n = 1:length(obj.current.slices)
                %
                % We use the "instances" list in SliceInfo to hold references
                % to the target instances. Note that the instances might not have
                % been read yet in the case of a circular reference to an
                % enclosing instance.
                %
                table = obj.current.indirectionTables{n};
                info = obj.current.slices{n};
                info.instances = cell(1, length(table));
                for j = 1:length(info.instances)
                    obj.addPatchEntry(table{j}, @(v) setInstance(info, j, v));
                end
            end

            r = Ice.SlicedData(obj.current.slices); % Makes a shallow copy
        end

        function r = resolveCompactId(obj, id)
            type = '';

            if ~isempty(obj.compactIdResolver)
                try
                    type = obj.compactIdResolver(id);
                catch ex
                    if isa(ex, 'Ice.LocalException')
                        rethrow(ex);
                    else
                        throw(Ice.MarshalException('', '', sprintf('exception in compact ID resolver for ID %d', id)));
                    end
                end
            end

            if isempty(type)
                prop = sprintf('IceCompactId.TypeId_%d.typeId', id);
                try
                    type = eval(prop);
                catch ex
                end
            end

            r = type;
        end

        function push(obj, sliceType)
            obj.current = IceInternal.EncapsDecoder11_InstanceData(obj.current);
            obj.current.sliceType = sliceType;
            obj.current.skipFirstSlice = false;
        end
    end
    properties(Access=private)
        compactIdResolver
        current
        valueIdIndex
        compactIdCache
    end
end
