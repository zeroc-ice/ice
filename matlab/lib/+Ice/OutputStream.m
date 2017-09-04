%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef OutputStream < Ice.WrapperObject
    methods
        function self = OutputStream(impl)
            self = self@Ice.WrapperObject(impl);
        end
        function writeBool(self, v)
            Ice.Util.callMethod(self, 'writeBool', v);
        end
        function writeBoolOpt(self, tag, v)
            Ice.Util.callMethod(self, 'writeBoolOpt', tag, v);
        end
        function writeBoolSeq(self, v)
            Ice.Util.callMethod(self, 'writeBoolSeq', v, length(v));
        end
        function writeBoolSeqOpt(self, tag, v)
            Ice.Util.callMethod(self, 'writeBoolSeqOpt', tag, v, length(v));
        end
        function writeByte(self, v)
            Ice.Util.callMethod(self, 'writeByte', v);
        end
        function writeByteOpt(self, tag, v)
            Ice.Util.callMethod(self, 'writeByteOpt', tag, v);
        end
        function writeByteSeq(self, v)
            Ice.Util.callMethod(self, 'writeByteSeq', v, length(v));
        end
        function writeByteSeqOpt(self, tag, v)
            Ice.Util.callMethod(self, 'writeByteSeqOpt', tag, v, length(v));
        end
        function writeShort(self, v)
            Ice.Util.callMethod(self, 'writeShort', v);
        end
        function writeShortOpt(self, tag, v)
            Ice.Util.callMethod(self, 'writeShortOpt', tag, v);
        end
        function writeShortSeq(self, v)
            Ice.Util.callMethod(self, 'writeShortSeq', v, length(v));
        end
        function writeShortSeqOpt(self, tag, v)
            Ice.Util.callMethod(self, 'writeShortSeqOpt', tag, v, length(v));
        end
        function writeInt(self, v)
            Ice.Util.callMethod(self, 'writeInt', v);
        end
        function writeIntOpt(self, tag, v)
            Ice.Util.callMethod(self, 'writeIntOpt', tag, v);
        end
        function writeIntSeq(self, v)
            Ice.Util.callMethod(self, 'writeIntSeq', v, length(v));
        end
        function writeIntSeqOpt(self, tag, v)
            Ice.Util.callMethod(self, 'writeIntSeqOpt', tag, v, length(v));
        end
        function writeLong(self, v)
            Ice.Util.callMethod(self, 'writeLong', v);
        end
        function writeLongOpt(self, tag, v)
            Ice.Util.callMethod(self, 'writeLongOpt', tag, v);
        end
        function writeLongSeq(self, v)
            Ice.Util.callMethod(self, 'writeLongSeq', v, length(v));
        end
        function writeLongSeqOpt(self, tag, v)
            Ice.Util.callMethod(self, 'writeLongSeqOpt', tag, v, length(v));
        end
        function writeFloat(self, v)
            Ice.Util.callMethod(self, 'writeFloat', v);
        end
        function writeFloatOpt(self, tag, v)
            Ice.Util.callMethod(self, 'writeFloatOpt', tag, v);
        end
        function writeFloatSeq(self, v)
            Ice.Util.callMethod(self, 'writeFloatSeq', v, length(v));
        end
        function writeFloatSeqOpt(self, tag, v)
            Ice.Util.callMethod(self, 'writeFloatSeqOpt', tag, v, length(v));
        end
        function writeDouble(self, v)
            Ice.Util.callMethod(self, 'writeDouble', v);
        end
        function writeDoubleOpt(self, tag, v)
            Ice.Util.callMethod(self, 'writeDoubleOpt', tag, v);
        end
        function writeDoubleSeq(self, v)
            Ice.Util.callMethod(self, 'writeDoubleSeq', v, length(v));
        end
        function writeDoubleSeqOpt(self, tag, v)
            Ice.Util.callMethod(self, 'writeDoubleSeqOpt', tag, v, length(v));
        end
        function writeString(self, str)
            Ice.Util.callMethod(self, 'writeString', str);
        end
        function writeStringOpt(self, tag, str)
            Ice.Util.callMethod(self, 'writeStringOpt', tag, str);
        end
        function writeStringSeq(self, v)
            Ice.Util.callMethod(self, 'writeStringSeq', v);
        end
        function writeStringSeqOpt(self, tag, v)
            Ice.Util.callMethod(self, 'writeStringSeqOpt', tag, v);
        end
        function writeSize(self, size)
            Ice.Util.callMethod(self, 'writeSize', size);
        end
        function writeProxy(self, v)
            if isempty(v)
                impl = libpointer;
            else
                impl = v.impl;
            end
            Ice.Util.callMethod(self, 'writeProxy', impl);
        end
        function writeProxyOpt(self, tag, v)
            if ~isempty(v)
                Ice.Util.callMethod(self, 'writeProxyOpt', tag, v.impl);
            end
        end
        function writeEnum(self, v, maxValue)
            Ice.Util.callMethod(self, 'writeEnum', v, maxValue);
        end
        function startEncapsulation(self)
            Ice.Util.callMethod(self, 'startEncapsulation');
        end
        function startEncapsulationWithFormat(self, fmt)
            Ice.Util.callMethod(self, 'startEncapsulationWithFormat', fmt);
        end
        function endEncapsulation(self)
            Ice.Util.callMethod(self, 'endEncapsulation');
        end
        function startSlice(self, typeId, compactId, last)
            Ice.Util.callMethod(self, 'startSlice', typeId, compactId, last);
        end
        function endSlice(self)
            Ice.Util.callMethod(self, 'endSlice');
        end
        function r = writeOptional(self, tag, f)
            supportsOptionals = libpointer('uint8Ptr', 0);
            Ice.Util.callMethod(self, 'writeOptional', tag, f, supportsOptionals);
            r = supportsOptionals.Value;
        end
        function r = startSize(self)
            pos = libpointer('uint32Ptr', 0);
            Ice.Util.callMethod(self, 'startSize', pos);
            r = pos.Value;
        end
        function endSize(self, pos)
            Ice.Util.callMethod(self, 'endSize', pos);
        end
    end
end
