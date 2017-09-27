%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef OutputStream < handle
    methods
        function obj = OutputStream(communicator, encoding)
            obj.communicator = communicator;
            obj.encoding = encoding;
            obj.encoding_1_0 = encoding.major == 1 && encoding.minor == 0;
            obj.encapsStack = [];
            obj.encapsCache = [];
            obj.buf = IceInternal.Buffer();
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
            sz = obj.buf.size;
            newSz = sz + 1;
            if newSz <= obj.buf.capacity
                obj.buf.buf(newSz) = uint8(v);
                obj.buf.size = newSz;
            else
                obj.buf.pushByte(uint8(v));
            end
        end
        function writeBoolOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.F1)
                obj.buf.pushByte(uint8(v));
            end
        end
        function writeBoolSeq(obj, v)
            len = length(v);
            obj.writeSize(len);
            if len > 0
                obj.buf.push(v);
            end
        end
        function writeBoolSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.VSize)
                obj.writeBoolSeq(v);
            end
        end
        function writeByte(obj, v)
            sz = obj.buf.size;
            newSz = sz + 1;
            if newSz <= obj.buf.capacity
                obj.buf.buf(newSz) = v;
                obj.buf.size = newSz;
            else
                obj.buf.pushByte(uint8(v));
            end
        end
        function writeByteOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.F1)
                obj.buf.pushByte(v);
            end
        end
        function writeByteSeq(obj, v)
            len = length(v);
            obj.writeSize(len);
            if len > 0
                obj.buf.push(v);
            end
        end
        function writeByteSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.VSize)
                obj.writeByteSeq(v);
            end
        end
        function writeShort(obj, v)
            sz = obj.buf.size;
            newSz = sz + 2;
            if newSz <= obj.buf.capacity
                obj.buf.size = newSz;
            else
                obj.buf.resize(newSz);
            end
            obj.buf.buf(sz + 1:newSz) = typecast(int16(v), 'uint8');
        end
        function writeShortOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.F2)
                obj.writeShort(v);
            end
        end
        function writeShortSeq(obj, v)
            len = length(v);
            obj.writeSize(len);
            if len > 0
                nbytes = len * 2;
                sz = obj.buf.size;
                obj.buf.resize(sz + nbytes);
                obj.buf.buf(sz + 1:sz + nbytes) = typecast(int16(v), 'uint8');
            end
        end
        function writeShortSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptionalVSize(tag, length(v), 2)
                obj.writeShortSeq(v);
            end
        end
        function writeInt(obj, v)
            sz = obj.buf.size;
            newSz = sz + 4;
            if newSz <= obj.buf.capacity
                obj.buf.size = newSz;
            else
                obj.buf.resize(newSz);
            end
            obj.buf.buf(sz + 1:newSz) = typecast(int32(v), 'uint8');
        end
        function writeIntOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.F4)
                obj.writeInt(v);
            end
        end
        function writeIntSeq(obj, v)
            len = length(v);
            obj.writeSize(len);
            if len > 0
                nbytes = len * 4;
                sz = obj.buf.size;
                obj.buf.resize(sz + nbytes);
                obj.buf.buf(sz + 1:sz + nbytes) = typecast(int32(v), 'uint8');
            end
        end
        function writeIntSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptionalVSize(tag, length(v), 4)
                obj.writeIntSeq(v);
            end
        end
        function writeLong(obj, v)
            sz = obj.buf.size;
            newSz = sz + 8;
            if newSz <= obj.buf.capacity
                obj.buf.size = newSz;
            else
                obj.buf.resize(newSz);
            end
            obj.buf.buf(sz + 1:newSz) = typecast(int64(v), 'uint8');
        end
        function writeLongOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.F8)
                obj.writeLong(v);
            end
        end
        function writeLongSeq(obj, v)
            len = length(v);
            obj.writeSize(len);
            if len > 0
                nbytes = len * 8;
                sz = obj.buf.size;
                obj.buf.resize(sz + nbytes);
                obj.buf.buf(sz + 1:sz + nbytes) = typecast(int64(v), 'uint8');
            end
        end
        function writeLongSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptionalVSize(tag, length(v), 8)
                obj.writeLongSeq(v);
            end
        end
        function writeFloat(obj, v)
            sz = obj.buf.size;
            newSz = sz + 4;
            if newSz <= obj.buf.capacity
                obj.buf.size = newSz;
            else
                obj.buf.resize(newSz);
            end
            obj.buf.buf(sz + 1:newSz) = typecast(single(v), 'uint8');
        end
        function writeFloatOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.F4)
                obj.writeFloat(v);
            end
        end
        function writeFloatSeq(obj, v)
            len = length(v);
            obj.writeSize(len);
            if len > 0
                nbytes = len * 4;
                sz = obj.buf.size;
                obj.buf.resize(sz + nbytes);
                obj.buf.buf(sz + 1:sz + nbytes) = typecast(single(v), 'uint8');
            end
        end
        function writeFloatSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptionalVSize(tag, length(v), 4)
                obj.writeFloatSeq(v);
            end
        end
        function writeDouble(obj, v)
            sz = obj.buf.size;
            newSz = sz + 8;
            if newSz <= obj.buf.capacity
                obj.buf.size = newSz;
            else
                obj.buf.resize(newSz);
            end
            obj.buf.buf(sz + 1:newSz) = typecast(double(v), 'uint8');
        end
        function writeDoubleOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.F8)
                obj.writeDouble(v);
            end
        end
        function writeDoubleSeq(obj, v)
            len = length(v);
            obj.writeSize(len);
            if len > 0
                nbytes = len * 8;
                sz = obj.buf.size;
                obj.buf.resize(sz + nbytes);
                obj.buf.buf(sz + 1:sz + nbytes) = typecast(double(v), 'uint8');
            end
        end
        function writeDoubleSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptionalVSize(tag, length(v), 8)
                obj.writeDoubleSeq(v);
            end
        end
        function writeString(obj, v)
            len = length(v);
            if len == 0
                sz = obj.buf.size;
                newSz = sz + 1;
                if newSz <= obj.buf.capacity
                    obj.buf.buf(newSz) = uint8(0);
                    obj.buf.size = newSz;
                else
                    obj.buf.pushByte(uint8(0));
                end
            else
                bytes = unicode2native(v, 'utf-8');
                obj.writeSize(length(bytes));
                obj.buf.push(bytes);
            end
        end
        function writeStringOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.VSize)
                obj.writeString(v);
            end
        end
        function writeStringSeq(obj, v)
            sz = length(v);
            obj.writeSize(sz);
            for i = 1:sz
                obj.writeString(v{i});
            end
        end
        function writeStringSeqOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.FSize)
                pos = obj.startSize();
                obj.writeStringSeq(v);
                obj.endSize(pos);
            end
        end
        function writeSize(obj, size)
            if size > 254
                sz = obj.buf.size;
                newSz = sz + 5;
                if newSz <= obj.buf.capacity
                    obj.buf.size = newSz;
                else
                    obj.buf.resize(newSz);
                end
                obj.buf.buf(sz + 1) = 255;
                obj.buf.buf(sz + 2:newSz) = typecast(int32(size), 'uint8');
            else
                sz = obj.buf.size;
                newSz = sz + 1;
                if newSz <= obj.buf.capacity
                    obj.buf.buf(newSz) = uint8(size);
                    obj.buf.size = newSz;
                else
                    obj.buf.pushByte(uint8(size));
                end
            end
        end
        function writeProxy(obj, v)
            if isempty(v)
                impl = libpointer('voidPtr');
            else
                impl = v.impl_;
            end
            bytes = IceInternal.Util.callWithResult('Ice_ObjectPrx_write', impl, obj.communicator.impl_, ...
                                                    obj.getEncoding());
            obj.buf.push(bytes);
        end
        function writeProxyOpt(obj, tag, v)
            if v ~= Ice.Unset && obj.writeOptional(tag, Ice.OptionalFormat.FSize)
                pos = obj.startSize();
                obj.writeProxy(v);
                obj.endSize(pos);
            end
        end
        function writeEnum(obj, v, maxValue)
            if obj.encoding_1_0
                if maxValue < 127
                    obj.writeByte(uint8(v));
                elseif maxValue < 32767
                    obj.writeShort(int16(v));
                else
                    obj.writeInt(v);
                end
            else
                obj.writeSize(v);
            end
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
            obj.encapsStack.start = obj.buf.size + 1; % Starting position of the encapsulation size

            obj.encoding_1_0 = encoding.major == 1 && encoding.minor == 0;

            %obj.writeInt(0); % Placeholder for the encapsulation length.
            sz = obj.buf.size;
            if sz + 6 <= obj.buf.capacity % Allocate enough for the size and the encoding
                obj.buf.size = sz + 6;
            else
                obj.buf.resize(sz + 6);
            end

            %Ice.EncodingVersion.ice_write(obj, obj.encapsStack.encoding);
            obj.buf.buf(sz + 5) = encoding.major;
            obj.buf.buf(sz + 6) = encoding.minor;
        end
        function endEncapsulation(obj)
            assert(~isempty(obj.encapsStack));

            % Size includes size and version.
            start = obj.encapsStack.start;
            sz = obj.buf.size - start + 1;
            %obj.rewriteInt(sz, start);
            obj.buf.buf(start:start + 3) = typecast(int32(sz), 'uint8');

            curr = obj.encapsStack;
            obj.encapsStack = curr.next;
            curr.next = obj.encapsCache;
            obj.encapsCache = curr;
            obj.encapsCache.encoder = [];

            if isempty(obj.encapsStack)
                obj.encoding_1_0 = obj.encoding.major == 1 && obj.encoding.minor == 0;
            else
                obj.encoding_1_0 = obj.encapsStack.encoding.major == 1 && obj.encapsStack.encoding.minor == 0;
            end
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
            elseif obj.encoding_1_0
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
            if obj.encoding_1_0
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
            r = obj.buf.size + 1;
            %obj.writeInt(0); % Placeholder for 32-bit size
            obj.buf.resize(obj.buf.size + 4);
        end
        function endSize(obj, pos)
            sz = obj.buf.size + 1;
            obj.buf.buf(pos:pos + 3) = typecast(int32(sz - pos - 4), 'uint8');
        end
        function r = getPos(obj)
            r = obj.buf.size;
        end
        function rewriteByte(obj, v, pos)
            obj.buf.buf(pos) = v;
        end
        function rewriteInt(obj, v, pos)
            obj.buf.buf(pos:pos + 3) = typecast(int32(v), 'uint8');
        end
        function writeBlob(obj, bytes)
            obj.buf.push(bytes);
        end
        function r = createInputStream(obj)
            buf = copy(obj.buf);
            r = Ice.InputStream(obj.communicator, obj.getEncoding(), buf);
        end
    end
    methods(Access=private)
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
                if obj.encoding_1_0
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
        encoding_1_0 logical
        format
        encapsStack
        encapsCache
    end
    properties
        buf
    end
end
