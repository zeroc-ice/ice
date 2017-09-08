%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef OutputStream < IceInternal.WrapperObject
    methods
        function obj = OutputStream(impl)
            obj = obj@IceInternal.WrapperObject(impl);
        end
        function writeBool(obj, v)
            Ice.Util.callMethod(obj, 'writeBool', v);
        end
        function writeBoolOpt(obj, tag, v)
            Ice.Util.callMethod(obj, 'writeBoolOpt', tag, v);
        end
        function writeBoolSeq(obj, v)
            Ice.Util.callMethod(obj, 'writeBoolSeq', v, length(v));
        end
        function writeBoolSeqOpt(obj, tag, v)
            Ice.Util.callMethod(obj, 'writeBoolSeqOpt', tag, v, length(v));
        end
        function writeByte(obj, v)
            Ice.Util.callMethod(obj, 'writeByte', v);
        end
        function writeByteOpt(obj, tag, v)
            Ice.Util.callMethod(obj, 'writeByteOpt', tag, v);
        end
        function writeByteSeq(obj, v)
            Ice.Util.callMethod(obj, 'writeByteSeq', v, length(v));
        end
        function writeByteSeqOpt(obj, tag, v)
            Ice.Util.callMethod(obj, 'writeByteSeqOpt', tag, v, length(v));
        end
        function writeShort(obj, v)
            Ice.Util.callMethod(obj, 'writeShort', v);
        end
        function writeShortOpt(obj, tag, v)
            Ice.Util.callMethod(obj, 'writeShortOpt', tag, v);
        end
        function writeShortSeq(obj, v)
            Ice.Util.callMethod(obj, 'writeShortSeq', v, length(v));
        end
        function writeShortSeqOpt(obj, tag, v)
            Ice.Util.callMethod(obj, 'writeShortSeqOpt', tag, v, length(v));
        end
        function writeInt(obj, v)
            Ice.Util.callMethod(obj, 'writeInt', v);
        end
        function writeIntOpt(obj, tag, v)
            Ice.Util.callMethod(obj, 'writeIntOpt', tag, v);
        end
        function writeIntSeq(obj, v)
            Ice.Util.callMethod(obj, 'writeIntSeq', v, length(v));
        end
        function writeIntSeqOpt(obj, tag, v)
            Ice.Util.callMethod(obj, 'writeIntSeqOpt', tag, v, length(v));
        end
        function writeLong(obj, v)
            Ice.Util.callMethod(obj, 'writeLong', v);
        end
        function writeLongOpt(obj, tag, v)
            Ice.Util.callMethod(obj, 'writeLongOpt', tag, v);
        end
        function writeLongSeq(obj, v)
            Ice.Util.callMethod(obj, 'writeLongSeq', v, length(v));
        end
        function writeLongSeqOpt(obj, tag, v)
            Ice.Util.callMethod(obj, 'writeLongSeqOpt', tag, v, length(v));
        end
        function writeFloat(obj, v)
            Ice.Util.callMethod(obj, 'writeFloat', v);
        end
        function writeFloatOpt(obj, tag, v)
            Ice.Util.callMethod(obj, 'writeFloatOpt', tag, v);
        end
        function writeFloatSeq(obj, v)
            Ice.Util.callMethod(obj, 'writeFloatSeq', v, length(v));
        end
        function writeFloatSeqOpt(obj, tag, v)
            Ice.Util.callMethod(obj, 'writeFloatSeqOpt', tag, v, length(v));
        end
        function writeDouble(obj, v)
            Ice.Util.callMethod(obj, 'writeDouble', v);
        end
        function writeDoubleOpt(obj, tag, v)
            Ice.Util.callMethod(obj, 'writeDoubleOpt', tag, v);
        end
        function writeDoubleSeq(obj, v)
            Ice.Util.callMethod(obj, 'writeDoubleSeq', v, length(v));
        end
        function writeDoubleSeqOpt(obj, tag, v)
            Ice.Util.callMethod(obj, 'writeDoubleSeqOpt', tag, v, length(v));
        end
        function writeString(obj, str)
            Ice.Util.callMethod(obj, 'writeString', str);
        end
        function writeStringOpt(obj, tag, str)
            Ice.Util.callMethod(obj, 'writeStringOpt', tag, str);
        end
        function writeStringSeq(obj, v)
            Ice.Util.callMethod(obj, 'writeStringSeq', v);
        end
        function writeStringSeqOpt(obj, tag, v)
            Ice.Util.callMethod(obj, 'writeStringSeqOpt', tag, v);
        end
        function writeSize(obj, size)
            Ice.Util.callMethod(obj, 'writeSize', size);
        end
        function writeProxy(obj, v)
            if isempty(v)
                impl = libpointer;
            else
                impl = v.impl;
            end
            Ice.Util.callMethod(obj, 'writeProxy', impl);
        end
        function writeProxyOpt(obj, tag, v)
            if ~isempty(v)
                Ice.Util.callMethod(obj, 'writeProxyOpt', tag, v.impl);
            end
        end
        function writeEnum(obj, v, maxValue)
            Ice.Util.callMethod(obj, 'writeEnum', v, maxValue);
        end
        function writeValue(obj, v)
            % Pass a reference to the wrapper object.
            Ice.Util.callMethod(obj, 'writeValue', obj, v);
        end
        function writeValueOpt(obj, tag, v)
            % Pass a reference to the wrapper object.
            Ice.Util.callMethod(obj, 'writeValueOpt', tag, obj, v);
        end
        function startEncapsulation(obj)
            Ice.Util.callMethod(obj, 'startEncapsulation');
        end
        function startEncapsulationWithFormat(obj, fmt)
            Ice.Util.callMethod(obj, 'startEncapsulationWithFormat', fmt);
        end
        function endEncapsulation(obj)
            Ice.Util.callMethod(obj, 'endEncapsulation');
        end
        function startSlice(obj, typeId, compactId, last)
            Ice.Util.callMethod(obj, 'startSlice', typeId, compactId, last);
        end
        function endSlice(obj)
            Ice.Util.callMethod(obj, 'endSlice');
        end
        function r = writeOptional(obj, tag, f)
            supportsOptionals = libpointer('uint8Ptr', 0);
            Ice.Util.callMethod(obj, 'writeOptional', tag, f, supportsOptionals);
            r = supportsOptionals.Value;
        end
        function r = startSize(obj)
            pos = libpointer('uint32Ptr', 0);
            Ice.Util.callMethod(obj, 'startSize', pos);
            r = pos.Value;
        end
        function endSize(obj, pos)
            Ice.Util.callMethod(obj, 'endSize', pos);
        end
    end
end
