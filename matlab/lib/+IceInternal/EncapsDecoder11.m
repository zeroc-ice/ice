classdef (Hidden) EncapsDecoder11 < IceInternal.EncapsDecoder
    % Copyright (c) ZeroC, Inc.

    methods
        function obj = EncapsDecoder11(is, encaps, classGraphDepthMax)
            obj@IceInternal.EncapsDecoder(is, encaps, classGraphDepthMax);
            obj.current = [];
            obj.valueIdIndex = 1;
        end

        function readValue(obj, cb)
            import IceInternal.Protocol;

            current = obj.current;

            index = obj.is.readSize();
            if index < 0
                throw(Ice.MarshalException('invalid object id'));
            elseif index == 0
                cb([]);
            elseif isobject(current) && bitand(current.sliceFlags, Protocol.FLAG_HAS_INDIRECTION_TABLE)
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
                if ~isConfigured(current.indirectPatchList) % Lazy initialization
                    current.indirectPatchList = configureDictionary('int32', 'IceInternal.IndirectPatchEntry');
                end
                e = IceInternal.IndirectPatchEntry();
                e.index = index; % MATLAB indexing starts at 1
                e.cb = cb;
                sz = current.indirectPatchList.numEntries;
                current.indirectPatchList(sz) = e;
            else
                obj.readInstance(index, cb);
            end
        end

        function throwException(obj)
            import IceInternal.Protocol;
            %assert(isempty(obj.current));

            % Inlining push()
            obj.current = IceInternal.EncapsDecoder11_InstanceData(obj.current);
            obj.current.sliceType = IceInternal.SliceType.ExceptionSlice;
            obj.current.skipFirstSlice = false;

            %
            % Read the first slice header.
            %
            obj.startSlice();
            mostDerivedId = obj.current.typeId;
            while true
                ex = obj.is.getCommunicator().getSliceLoader().newInstance(obj.current.typeId);

                if ~isempty(ex)
                    %
                    % Exceptions are value types so we have to replace 'ex' with its new value after calling methods.
                    %
                    ex = ex.iceRead(obj.is);
                    ex = ex.icePostUnmarshal();
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
                        throw(Ice.MarshalException(sprintf('unknown exception type ''%s''', mostDerivedId)));
                    end

                    obj.startSlice();
                end
            end
        end

        function startInstance(obj, ~)
            %assert(obj.current.sliceType == sliceType);
            obj.current.skipFirstSlice = true;
        end

        function r = endInstance(obj)
            slicedData = obj.readSlicedData();
            current = obj.current;
            current.slices = {};
            current.indirectionTables = {};
            current.indirectPatchList = dictionary; % unconfigured dictionary
            obj.current = current.previous;
            r = slicedData;
        end

        function startSlice(obj)
            import IceInternal.Protocol;
            %
            % If first slice, don't read the header, it was already read in
            % readInstance or throwException to find the factory.
            %
            current = obj.current;
            if current.skipFirstSlice
                current.skipFirstSlice = false;
                return;
            end

            is = obj.is;
            current.sliceFlags = is.readByte();

            %
            % Read the type ID, for value slices the type ID is encoded as a
            % string or as an index, for exceptions it's always encoded as a
            % string.
            %
            if current.sliceType == IceInternal.SliceType.ValueSlice
                % Must be checked first!
                if bitand(current.sliceFlags, Protocol.FLAG_HAS_TYPE_ID_COMPACT) == Protocol.FLAG_HAS_TYPE_ID_COMPACT
                    current.typeIdIndex = -1;
                    current.compactId = is.readSize();
                    current.typeId = int2str(current.compactId);
                elseif bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_TYPE_ID_INDEX)
                    typeIdIndex = is.readSize();
                    current.typeIdIndex = typeIdIndex;
                    current.typeId = obj.getTypeId(typeIdIndex);
                    current.compactId = -1;
                elseif bitand(obj.current.sliceFlags, Protocol.FLAG_HAS_TYPE_ID_STRING)
                    [current.typeIdIndex, current.typeId] = obj.readTypeId();
                    current.compactId = -1;
                else
                    % Only the most derived slice encodes the type ID for the compact format.
                    current.typeId = '';
                    current.typeIdIndex = -1;
                    current.compactId = -1;
                end
            else
                current.typeId = is.readString();
                current.compactId = -1;
            end

            %
            % Read the slice size if necessary.
            %
            if bitand(current.sliceFlags, Protocol.FLAG_HAS_SLICE_SIZE)
                current.sliceSize = is.readInt();
                if current.sliceSize < 4
                    throw(Ice.MarshalException('invalid slice size'));
                end
            else
                current.sliceSize = 0;
            end
        end

        function endSlice(obj)
            import IceInternal.Protocol;
            current = obj.current;
            is = obj.is;
            if bitand(current.sliceFlags, Protocol.FLAG_HAS_OPTIONAL_MEMBERS)
                is.skipOptionals();
            end

            %
            % Read the indirection table if one is present and transform the
            % indirect patch list into patch entries with direct references.
            %
            if bitand(current.sliceFlags, Protocol.FLAG_HAS_INDIRECTION_TABLE)
                %
                % The table is written as a sequence<size> to conserve space.
                %
                sz = is.readAndCheckSeqSize(1);
                indirectionTable = cell(1, sz);
                for i = 1:sz
                    indirectionTable{i} = obj.readInstance(is.readSize(), []);
                end

                %
                % Sanity checks. If there are optional members, it's possible
                % that not all instance references were read if they are from
                % unknown optional data members.
                %
                if isempty(indirectionTable)
                    throw(Ice.MarshalException('empty indirection table'));
                end
                if ~isConfigured(current.indirectPatchList) && ...
                   bitand(current.sliceFlags, Protocol.FLAG_HAS_OPTIONAL_MEMBERS) == 0
                    throw(Ice.MarshalException('no references to indirection table'));
                end

                %
                % Convert indirect references into direct references.
                %
                if isConfigured(current.indirectPatchList)
                    keys = current.indirectPatchList.keys;
                    for i = 1:length(keys)
                        e = current.indirectPatchList(keys(i));
                        %assert(e.index > 0); % MATLAB starts indexing at 1
                        if e.index > length(indirectionTable)
                            throw(Ice.MarshalException('indirection out of range'));
                        end
                        obj.addPatchEntry(indirectionTable{e.index}, e.cb);
                    end
                    current.indirectPatchList = dictionary; % unconfigured dictionary
                end
            end
        end

        function skipSlice(obj)
            import IceInternal.Protocol;

            start = obj.is.getPos();
            current = obj.current;
            is = obj.is;
            if bitand(current.sliceFlags, Protocol.FLAG_HAS_SLICE_SIZE)
                %assert(obj.current.sliceSize >= 4);
                is.skip(current.sliceSize - 4);
            else
                if current.sliceType == IceInternal.SliceType.ValueSlice
                    reason = sprintf('The Slice loader did not find a class for type ID ''%s''.', current.typeId);
                    throw(Ice.MarshalException(reason));
                else
                    reason = sprintf('The Slice loader did not find a user exception class for type ID ''%s''.', current.typeId);
                    throw(Ice.MarshalException(reason));
                end
            end

            %
            % Preserve this slice if unmarshalling a value in Slice format. Exception slices are not preserved.
            %

            if current.sliceType == IceInternal.SliceType.ValueSlice

                hasOptionalMembers = bitand(current.sliceFlags, Protocol.FLAG_HAS_OPTIONAL_MEMBERS) > 0;
                if hasOptionalMembers
                    %
                    % Don't include the optional member end marker. It will be re-written by
                    % endSlice when the sliced data is re-written.
                    %
                    bytes = is.getBytes(start, is.getPos() - 2);
                else
                    bytes = is.getBytes(start, is.getPos() - 1);
                end
                info = Ice.SliceInfo(current.typeId, current.compactId, bytes, hasOptionalMembers,...
                    bitand(current.sliceFlags, Protocol.FLAG_IS_LAST_SLICE) > 0);

                current.slices{end + 1} = info;
            end

            %
            % Read the indirect instance table. We read the instances or their
            % IDs if the instance is a reference to an already unmarshaled
            % object.
            %
            % The SliceInfo object sequence is initialized only if
            % readSlicedData is called.
            %
            if bitand(current.sliceFlags, Protocol.FLAG_HAS_INDIRECTION_TABLE)
                sz = is.readAndCheckSeqSize(1);
                indirectionTable = cell(1, sz);
                for i = 1:sz
                    indirectionTable{i} = obj.readInstance(is.readSize(), []);
                end
                current.indirectionTables{end + 1} = indirectionTable;
            else
                current.indirectionTables{end + 1} = {};
            end
        end
    end
    methods (Access = private)
        function r = readInstance(obj, index, cb)
            import IceInternal.Protocol;
            %assert(index > 0);

            if index > 1
                if ~isempty(cb)
                    obj.addPatchEntry(index, cb);
                end
                r = index;
                return;
            end

            % Inlining push()
            obj.current = IceInternal.EncapsDecoder11_InstanceData(obj.current);
            current = obj.current;
            current.sliceType = IceInternal.SliceType.ValueSlice;
            current.skipFirstSlice = false;

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
            mostDerivedId = current.typeId;
            while true
                if ~isempty(current.typeId)
                    v = obj.newInstance(current.typeId);
                    if isobject(v)
                        %
                        % We have an instance, get out of this loop.
                        %
                        break;
                    end
                end

                %
                % Slice off what we don't understand.
                %
                obj.skipSlice();

                %
                % If this is the last slice, keep the instance as an opaque
                % UnknownSlicedValue object.
                %
                if bitand(current.sliceFlags, Protocol.FLAG_IS_LAST_SLICE)
                    %
                    % Provide a factory with an opportunity to supply the instance.
                    % We pass the "::Ice::Object" ID to indicate that this is the
                    % last chance to preserve the instance.
                    %
                    v = obj.newInstance(Ice.Value.ice_staticId());
                    if ~isobject(v)
                        v = Ice.UnknownSlicedValue(mostDerivedId);
                    end
                    break;
                end

                obj.startSlice(); % Read next Slice header for next iteration.
            end

            assert(isobject(v));

            obj.classGraphDepth = obj.classGraphDepth + 1;
            if obj.classGraphDepth > obj.classGraphDepthMax
                throw(Ice.MarshalException('maximum class graph depth reached'))
            end

            %
            % Unmarshal the instance.
            %
            obj.unmarshal(index, v);

            obj.classGraphDepth = obj.classGraphDepth - 1;

            if ~isobject(obj.current) && obj.patchMapLength > 0
                %
                % If any entries remain in the patch map, the sender has sent an index for an instance, but failed
                % to supply the instance.
                %
                throw(Ice.MarshalException('index for class received, but no instance'));
            end

            if ~isempty(cb)
                cb(v);
            end

            r = index;
        end

        function r = readSlicedData(obj)
            current = obj.current;
            if isempty(current.slices) % No preserved slices.
                r = [];
                return;
            end

            %
            % The _indirectionTables member holds the indirection table for each slice
            % in _slices.
            %
            %assert(length(current.slices) == length(current.indirectionTables));
            function setInstance(si, n, v)
                si.instances{n} = v;
            end
            for n = 1:length(current.slices)
                %
                % We use the "instances" list in SliceInfo to hold references
                % to the target instances. Note that the instances might not have
                % been read yet in the case of a circular reference to an
                % enclosing instance.
                %
                table = current.indirectionTables{n};
                info = current.slices{n};
                info.instances = cell(1, length(table));
                for j = 1:length(info.instances)
                    obj.addPatchEntry(table{j}, @(v) setInstance(info, j, v));
                end
            end

            r = Ice.SlicedData(current.slices); % Makes a shallow copy
        end
    end
    properties (Access = public)
        current
    end
    properties (Access = private)
        valueIdIndex
    end
end
