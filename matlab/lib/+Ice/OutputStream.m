%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef OutputStream < IceInternal.WrapperObject
    methods
        function obj = OutputStream(impl, communicator)
            obj = obj@IceInternal.WrapperObject(impl);
            obj.communicator = communicator;
            obj.encoding = obj.callWithResult_('getEncoding');
            obj.encapsStack = [];
            obj.encapsCache = [];
        end
        function r = getCommunicator(obj)
            r = obj.communicator;
        end
        function r = getEncoding(obj)
            if isempty(obj.encapsStack)
                r = obj.encoding;
            else
                r = obj.encapsStack.encoding;
            end
        end
        function setFormat(obj, format)
            obj.format = format;
        end
        function writeBool(obj, v)
            obj.call_('writeBool', v);
        end
        function writeBoolOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.F1)
                obj.call_('writeBool', v);
            end
        end
        function writeBoolSeq(obj, v)
            obj.call_('writeBoolSeq', v, length(v));
        end
        function writeBoolSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.VSize)
                obj.call_('writeBoolSeq', v, length(v));
            end
        end
        function writeByte(obj, v)
            obj.call_('writeByte', v);
        end
        function writeByteOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.F1)
                obj.call_('writeByte', v);
            end
        end
        function writeByteSeq(obj, v)
            obj.call_('writeByteSeq', v, length(v));
        end
        function writeByteSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.VSize)
                obj.call_('writeByteSeq', v, length(v));
            end
        end
        function writeShort(obj, v)
            obj.call_('writeShort', v);
        end
        function writeShortOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.F2)
                obj.call_('writeShort', v);
            end
        end
        function writeShortSeq(obj, v)
            obj.call_('writeShortSeq', v, length(v));
        end
        function writeShortSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptionalVSize(tag, length(v), 2)
                obj.call_('writeShortSeq', v, length(v));
            end
        end
        function writeInt(obj, v)
            obj.call_('writeInt', v);
        end
        function writeIntOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.F4)
                obj.call_('writeInt', v);
            end
        end
        function writeIntSeq(obj, v)
            obj.call_('writeIntSeq', v, length(v));
        end
        function writeIntSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptionalVSize(tag, length(v), 4)
                obj.call_('writeIntSeq', v, length(v));
            end
        end
        function writeLong(obj, v)
            obj.call_('writeLong', v);
        end
        function writeLongOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.F8)
                obj.call_('writeLong', v);
            end
        end
        function writeLongSeq(obj, v)
            obj.call_('writeLongSeq', v, length(v));
        end
        function writeLongSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptionalVSize(tag, length(v), 8)
                obj.call_('writeLongSeq', v, length(v));
            end
        end
        function writeFloat(obj, v)
            obj.call_('writeFloat', v);
        end
        function writeFloatOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.F4)
                obj.call_('writeFloat', v);
            end
        end
        function writeFloatSeq(obj, v)
            obj.call_('writeFloatSeq', v, length(v));
        end
        function writeFloatSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptionalVSize(tag, length(v), 4)
                obj.call_('writeFloatSeq', v, length(v));
            end
        end
        function writeDouble(obj, v)
            obj.call_('writeDouble', v);
        end
        function writeDoubleOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.F8)
                obj.call_('writeDouble', v);
            end
        end
        function writeDoubleSeq(obj, v)
            obj.call_('writeDoubleSeq', v, length(v));
        end
        function writeDoubleSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptionalVSize(tag, length(v), 8)
                obj.call_('writeDoubleSeq', v, length(v));
            end
        end
        function writeString(obj, v)
            obj.call_('writeString', v);
        end
        function writeStringOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.VSize)
                obj.call_('writeString', v);
            end
        end
        function writeStringSeq(obj, v)
            obj.call_('writeStringSeq', v);
        end
        function writeStringSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.FSize)
                pos = obj.startSize();
                obj.call_('writeStringSeq', v);
                obj.endSize(pos);
            end
        end
        function writeSize(obj, size)
            obj.call_('writeSize', size);
        end
        function writeProxy(obj, v)
            if isempty(v)
                impl = libpointer;
            else
                impl = v.impl_;
            end
            obj.call_('writeProxy', impl);
        end
        function writeProxyOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.FSize)
                pos = obj.startSize();
                obj.writeProxy(v);
                obj.endSize(pos);
            end
        end
        function writeEnum(obj, v, maxValue)
            obj.call_('writeEnum', v, maxValue);
        end
        function writeValue(obj, v)
            if isempty(obj.format)
                % Lazy initialization
                if obj.communicator.getProperties().getPropertyAsIntWithDefault('Ice.Default.SlicedFormat', 0) > 0
                    obj.format = Ice.FormatType.SlicedFormat;
                else
                    obj.format = Ice.FormatType.CompactFormat;
                end
            end

            obj.initEncaps();
            obj.encapsStack.encoder.writeValue(v);
        end
        function writeValueOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.Class)
                obj.writeValue(v);
            end
        end
        function startValue(obj, slicedData)
            assert(~isempty(obj.encapsStack) && ~isempty(obj.encapsStack.encoder));
            obj.encapsStack.encoder.startInstance(IceInternal.SliceType.ValueSlice, slicedData);
        end
        function endValue(obj)
            assert(~isempty(obj.encapsStack) && ~isempty(obj.encapsStack.encoder));
            obj.encapsStack.encoder.endInstance();
        end
        function startEncapsulation(obj, format)
            %
            % If no encoding version is specified, use the current write
            % encapsulation encoding version if there's a current write
            % encapsulation, otherwise, use the stream encoding version.
            %

            encoding = [];
            if ~isempty(obj.encapsStack)
                encoding = obj.encapsStack.encoding;
                if isempty(format)
                    format = obj.encapsStack.format;
                end
            else
                encoding = obj.encoding;
                if isempty(format)
                    format = Ice.FormatType.DefaultFormat;
                end
            end

            curr = obj.encapsCache;
            if ~isempty(curr)
                curr.encoder = [];
                obj.encapsCache = obj.encapsCache.next;
            else
                curr = IceInternal.WriteEncaps();
            end
            curr.next = obj.encapsStack;
            obj.encapsStack = curr;

            obj.encapsStack.format = format;
            obj.encapsStack.encoding = encoding;
            obj.encapsStack.start = obj.pos();

            obj.writeInt(0); % Placeholder for the encapsulation length.
            Ice.EncodingVersion.ice_write(obj, obj.encapsStack.encoding);
        end
        function endEncapsulation(obj)
            assert(~isempty(obj.encapsStack));

            % Size includes size and version.
            start = obj.encapsStack.start;
            sz = obj.pos() - start;
            obj.rewriteInt(sz, start);

            curr = obj.encapsStack;
            obj.encapsStack = curr.next;
            curr.next = obj.encapsCache;
            obj.encapsCache = curr;
            obj.encapsCache.encoder = [];
        end
        function startSlice(obj, typeId, compactId, last)
            assert(~isempty(obj.encapsStack) && ~isempty(obj.encapsStack.encoder));
            obj.encapsStack.encoder.startSlice(typeId, compactId, last);
        end
        function endSlice(obj)
            assert(~isempty(obj.encapsStack) && ~isempty(obj.encapsStack.encoder));
            obj.encapsStack.encoder.endSlice();
        end
        function writePendingValues(obj)
            if ~isempty(obj.encapsStack) && ~isempty(obj.encapsStack.encoder)
                obj.encapsStack.encoder.writePendingValues();
            elseif isequal(obj.getEncoding(), IceInternal.Protocol.Encoding_1_0)
                %
                % If using the 1.0 encoding and no instances were written, we
                % still write an empty sequence for pending instances if
                % requested (i.e.: if this is called).
                %
                % This is required by the 1.0 encoding, even if no instances
                % are written we do marshal an empty sequence if marshaled
                % data types use classes.
                %
                obj.writeSize(0);
            end
        end
        function r = writeOptional(obj, tag, format)
            assert(~isempty(obj.encapsStack));
            if ~isempty(obj.encapsStack.encoder)
                r = obj.encapsStack.encoder.writeOptional(tag, format);
            else
                r = obj.writeOptionalImpl(tag, format);
            end
        end
        function r = writeOptionalImpl(obj, tag, format)
            if isequal(obj.getEncoding(), IceInternal.Protocol.Encoding_1_0)
                r = false; % Optional members aren't supported with the 1.0 encoding.
                return;
            end

            v = uint8(format); % One of Ice.OptionalFormat...
            if tag < 30
                v = bitor(v, bitshift(tag, 3));
                obj.writeByte(v);
            else
                v = bitor(v, 240); % tag = 30
                obj.writeByte(v);
                obj.writeSize(tag);
            end
            r = true;
        end
        function r = startSize(obj)
            pos = libpointer('uint32Ptr', 0);
            obj.call_('startSize', pos);
            r = pos.Value;
        end
        function endSize(obj, pos)
            obj.call_('endSize', pos);
        end
        function r = pos(obj)
            p = libpointer('uint32Ptr', 0);
            obj.call_('pos', p);
            r = p.Value;
        end
        function rewriteByte(obj, v, pos)
            obj.call_('rewriteByte', v, pos);
        end
        function rewriteInt(obj, v, pos)
            obj.call_('rewriteInt', v, pos);
        end
        function writeBlob(obj, bytes)
            obj.call_('writeBlob', bytes, length(bytes));
        end
    end
    methods(Access=private)
        function r = isEncoding_1_0(obj)
            if ~isempty(obj.encapsStack)
                r = isequal(obj.encapsStack.encoding, IceInternal.Protocol.Encoding_1_0);
            else
                r = isequal(obj.encoding, IceInternal.Protocol.Encoding_1_0);
            end
        end
        function initEncaps(obj)
            if isempty(obj.encapsStack) % Lazy initialization
                obj.encapsStack = obj.encapsCache;
                if ~isempty(obj.encapsStack)
                    obj.encapsCache = obj.encapsCache.next;
                else
                    obj.encapsStack = IceInternal.WriteEncaps();
                end
                obj.encapsStack.encoding = obj.encoding;
            end

            if obj.encapsStack.format == Ice.FormatType.DefaultFormat
                obj.encapsStack.format = obj.format;
            end

            if isempty(obj.encapsStack.encoder) % Lazy initialization.
                if isequal(obj.getEncoding(), IceInternal.Protocol.Encoding_1_0)
                    obj.encapsStack.encoder = IceInternal.EncapsEncoder10(obj, obj.encapsStack);
                else
                    obj.encapsStack.encoder = IceInternal.EncapsEncoder11(obj, obj.encapsStack);
                end
            end
        end
        function r = writeOptionalVSize(obj, tag, len, elemSize)
            r = false;
            if obj.writeOptional(tag, Ice.OptionalFormat.VSize)
                if len == 0
                    obj.writeSize(1);
                    obj.writeSize(0);
                else
                    sz = len * elemSize;
                    if len > 254
                        sz = sz + 5;
                    else
                        sz = sz + 1;
                    end
                    obj.writeSize(sz);
                    r = true;
                end
            end
        end
    end
    properties(Access=private)
        communicator
        encoding
        format
        encapsStack
        encapsCache
    end
end
