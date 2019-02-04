%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

%
% These tests exercise some corner cases of the mapping.
%
classdef LocalTests
    methods(Static)
        function localTests(helper)
            import LocalTest.*;

            communicator = helper.communicator();
            props = communicator.getProperties();

            fprintf('testing class members locally... ');

            if props.getPropertyAsIntWithDefault('Ice.Default.SlicedFormat', 0) == 1
                format = Ice.FormatType.SlicedFormat;
            else
                format = Ice.FormatType.CompactFormat;
            end

            if strcmp(props.getPropertyWithDefault('Ice.Default.EncodingVersion', '1.1'), '1.1')
                encoding = Ice.EncodingVersion(1, 1);
            else
                encoding = Ice.EncodingVersion(1, 0);
            end

            %
            % Test: struct containing class member. The struct is a value type and the generated code needs
            % to "convert" it to finalize the class instances.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            s = S1(C1(3));
            S1.ice_write(out, s);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = S1.ice_read(is);
            is.readPendingValues();
            is.endEncapsulation();
            % Prior to conversion, the member is a ValueHolder
            assert(isa(r.c1, 'IceInternal.ValueHolder'));
            r = r.ice_convert();
            % Now the member should be an instance of the class
            assert(isa(r.c1, symbol('C1')));

            %
            % Test: sequence of class. The generated code temporarily uses an Ice.CellArrayHandle to store the
            % class instances. Calling convert() returns the wrapped cell array.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            seq = {};
            for i = 1:10
                seq{i} = C1(i);
            end
            C1Seq.write(out, seq);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = C1Seq.read(is);
            is.readPendingValues();
            is.endEncapsulation();
            % Prior to conversion, the result is a CellArrayHandle
            assert(isa(r, 'IceInternal.CellArrayHandle'));
            r = C1Seq.convert(r);
            % Now the result should be a cell arary
            assert(isa(r, 'cell'));
            assert(length(r) == length(seq));
            for i = 1:10
                assert(r{i}.i == i);
            end

            %
            % Test: sequence of struct containing class. Each element of the sequence needs to be converted.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            seq = S1();
            for i = 1:10
                seq(i) = S1(C1(i));
            end
            S1Seq.write(out, seq);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = S1Seq.read(is);
            is.readPendingValues();
            is.endEncapsulation();
            % The result should be an array of S1
            assert(isa(r, symbol('S1')));
            assert(length(r) == length(seq));
            % The structures haven't been converted yet
            assert(isa(r(1).c1, 'IceInternal.ValueHolder'));
            r = S1Seq.convert(r);
            assert(isa(r(1).c1, symbol('C1')));
            for i = 1:10
                assert(r(i).c1.i == i);
            end

            %
            % Test: dictionary with class value type. The map temporarily stores IceInternal.ValueHolder objects as its
            % values until converted.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            d = containers.Map('KeyType', 'int32', 'ValueType', 'any');
            for i = 1:10
                d(i) = C1(i);
            end
            C1Dict.write(out, d);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = C1Dict.read(is);
            is.readPendingValues();
            is.endEncapsulation();
            % The result should be a map
            assert(isa(r, 'containers.Map'));
            assert(length(r) == length(d));
            % The entry values haven't been converted yet
            assert(isa(r(1), 'IceInternal.ValueHolder'));
            r = C1Dict.convert(r);
            assert(isa(r(1), symbol('C1')));
            for i = 1:10
                assert(r(i).i == i);
            end

            %
            % Test: dictionary with structure value type containing a class member. The structure temporarily stores
            % IceInternal.ValueHolder objects as its member until converted.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            d = containers.Map('KeyType', 'int32', 'ValueType', 'any');
            for i = 1:10
                d(i) = S1(C1(i));
            end
            S1Dict.write(out, d);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = S1Dict.read(is);
            is.readPendingValues();
            is.endEncapsulation();
            % The result should be a map
            assert(isa(r, 'containers.Map'));
            assert(length(r) == length(d));
            % The entry values haven't been converted yet
            assert(isa(r(1).c1, 'IceInternal.ValueHolder'));
            r = S1Dict.convert(r);
            assert(isa(r(1).c1, symbol('C1')));
            for i = 1:10
                assert(r(i).c1.i == i);
            end

            %
            % Test: sequence of sequence of class. Converting the outer sequence should convert every inner sequence.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            seq = {};
            for i = 1:10
                for j = 1:5
                    seq{i}{j} = C1(i * 10 + j);
                end
            end
            C1SeqSeq.write(out, seq);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = C1SeqSeq.read(is);
            is.readPendingValues();
            is.endEncapsulation();
            assert(isa(r, 'cell'));
            assert(isa(r{1}, 'IceInternal.CellArrayHandle'));
            r = C1SeqSeq.convert(r);
            assert(isa(r{1}, 'cell'));
            assert(length(r) == length(seq));
            for i = 1:10
                assert(length(r{i}) == 5);
                for j = 1:5
                    assert(r{i}{j}.i == i * 10 + j);
                end
            end

            %
            % Test: sequence of sequence of struct containing class. Each element of the inner sequence needs to
            % be converted.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            seq = {};
            for i = 1:10
                seq{i} = S1();
                for j = 1:5
                    seq{i}(j) = S1(C1(i * 10 + j));
                end
            end
            S1SeqSeq.write(out, seq);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = S1SeqSeq.read(is);
            is.readPendingValues();
            is.endEncapsulation();
            assert(isa(r, 'cell'));
            assert(isa(r{1}, symbol('S1')));
            assert(length(r) == length(seq));
            % The structures haven't been converted yet
            assert(isa(r{1}(1).c1, 'IceInternal.ValueHolder'));
            r = S1SeqSeq.convert(r);
            assert(isa(r{1}(1).c1, symbol('C1')));
            for i = 1:10
                assert(length(r{i}) == 5);
                for j = 1:5
                    assert(r{i}(j).c1.i == i * 10 + j);
                end
            end

            %
            % Test: struct containing struct containing class member. The inner struct must be converted.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            s = S2(S1(C1(3)));
            S2.ice_write(out, s);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = S2.ice_read(is);
            is.readPendingValues();
            is.endEncapsulation();
            % Prior to conversion, the class member is a ValueHolder
            assert(isa(r.s1.c1, 'IceInternal.ValueHolder'));
            r = r.ice_convert();
            % Now the member should be an instance of the class
            assert(isa(r.s1.c1, symbol('C1')));

            %
            % Test: struct containing sequence of class. The sequence must be converted.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            s3 = S3();
            for i = 1:10
                s3.c1seq{i} = C1(i);
            end
            S3.ice_write(out, s3);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = S3.ice_read(is);
            is.readPendingValues();
            is.endEncapsulation();
            % Prior to conversion, the sequence is a CellArrayHandle
            assert(isa(r.c1seq, 'IceInternal.CellArrayHandle'));
            r = r.ice_convert();
            % Now the result should be a cell arary
            assert(isa(r.c1seq, 'cell'));
            assert(length(r.c1seq) == length(s3.c1seq));
            for i = 1:10
                assert(r.c1seq{i}.i == i);
            end

            %
            % Test: struct containing sequence of struct containing class. Each element of the sequence needs to
            % be converted.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            s4 = S4();
            s4.s1seq = S1();
            for i = 1:10
                s4.s1seq(i) = S1(C1(i));
            end
            S4.ice_write(out, s4);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = S4.ice_read(is);
            is.readPendingValues();
            is.endEncapsulation();
            % The sequence should be an array of S1
            assert(isa(r.s1seq, symbol('S1')));
            assert(length(r.s1seq) == length(s4.s1seq));
            % The structures haven't been converted yet
            assert(isa(r.s1seq(1).c1, 'IceInternal.ValueHolder'));
            r = r.ice_convert();
            assert(isa(r.s1seq(1).c1, symbol('C1')));
            for i = 1:10
                assert(r.s1seq(i).c1.i == i);
            end

            %
            % Test: struct containing dictionary with class value type. The map temporarily stores IceInternal.ValueHolder
            % objects as its values until converted.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            s5 = S5();
            for i = 1:10
                s5.c1dict(i) = C1(i);
            end
            S5.ice_write(out, s5);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = S5.ice_read(is);
            is.readPendingValues();
            is.endEncapsulation();
            % The member should be a map
            assert(isa(r.c1dict, 'containers.Map'));
            assert(length(r.c1dict) == length(s5.c1dict));
            % The entry values haven't been converted yet
            assert(isa(r.c1dict(1), 'IceInternal.ValueHolder'));
            r = r.ice_convert();
            assert(isa(r.c1dict(1), symbol('C1')));
            for i = 1:10
                assert(r.c1dict(i).i == i);
            end

            %
            % Test: struct containing dictionary with structure value type containing a class member. The inner
            % structure temporarily stores IceInternal.ValueHolder objects as its member until converted.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            s6 = S6();
            for i = 1:10
                s6.s1dict(i) = S1(C1(i));
            end
            S6.ice_write(out, s6);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = S6.ice_read(is);
            is.readPendingValues();
            is.endEncapsulation();
            % The member should be a map
            assert(isa(r.s1dict, 'containers.Map'));
            assert(length(r.s1dict) == length(s6.s1dict));
            % The entry values haven't been converted yet
            assert(isa(r.s1dict(1).c1, 'IceInternal.ValueHolder'));
            r = r.ice_convert();
            assert(isa(r.s1dict(1).c1, symbol('C1')));
            for i = 1:10
                assert(r.s1dict(i).c1.i == i);
            end

            %
            % Test: struct containing sequence of sequence of class. Converting the outer sequence should convert
            % every inner sequence.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            s7 = S7();
            for i = 1:10
                for j = 1:5
                    s7.c1seqseq{i}{j} = C1(i * 10 + j);
                end
            end
            S7.ice_write(out, s7);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = S7.ice_read(is);
            is.readPendingValues();
            is.endEncapsulation();
            assert(isa(r.c1seqseq, 'cell'));
            assert(isa(r.c1seqseq{1}, 'IceInternal.CellArrayHandle'));
            r = r.ice_convert();
            assert(isa(r.c1seqseq{1}, 'cell'));
            assert(length(r.c1seqseq) == length(s7.c1seqseq));
            for i = 1:10
                assert(length(r.c1seqseq{i}) == 5);
                for j = 1:5
                    assert(r.c1seqseq{i}{j}.i == i * 10 + j);
                end
            end

            %
            % Test: struct containing sequence of sequence of struct containing class. Each element of the inner
            % sequence needs to be converted.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            s8 = S8();
            for i = 1:10
                s8.s1seqseq{i} = S1();
                for j = 1:5
                    s8.s1seqseq{i}(j) = S1(C1(i * 10 + j));
                end
            end
            S8.ice_write(out, s8);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = S8.ice_read(is);
            is.readPendingValues();
            is.endEncapsulation();
            assert(isa(r.s1seqseq, 'cell'));
            assert(isa(r.s1seqseq{1}, symbol('S1')));
            assert(length(r.s1seqseq) == length(s8.s1seqseq));
            % The structures haven't been converted yet
            assert(isa(r.s1seqseq{1}(1).c1, 'IceInternal.ValueHolder'));
            r = r.ice_convert();
            assert(isa(r.s1seqseq{1}(1).c1, symbol('C1')));
            for i = 1:10
                assert(length(r.s1seqseq{i}) == 5);
                for j = 1:5
                    assert(r.s1seqseq{i}(j).c1.i == i * 10 + j);
                end
            end

            %
            % Test: class containing struct containing class. The ice_postUnmarshal callback should be postponed
            % until after the class has converted its member.
            %
            % We installed a factory that creates a subclass of the outer class and overrides ice_postUnmarshal.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            cb = CB1(S1(C1(3)));
            out.writeValue(cb);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            h = IceInternal.ValueHolder();
            is.readValue(@(v) h.set(v), symbol('CB1'));
            is.readPendingValues();
            % At this point, h.value should hold the instance, but ice_postUnmarshal should not have been called yet.
            assert(~isempty(h.value));
            assert(~h.value.postUnmarshalInvoked);
            % Prior to conversion, the member is a ValueHolder
            assert(isa(h.value.s1.c1, 'IceInternal.ValueHolder'));
            % Ending the encapsulation should trigger the conversion and the ice_postUnmarshal callback
            is.endEncapsulation();
            % Now the member should be an instance of the class
            assert(isa(h.value.s1.c1, symbol('C1')));
            assert(h.value.postUnmarshalInvoked);

            %
            % Test: class containing sequence of class. The ice_postUnmarshal callback should be postponed
            % until after the class has converted its member.
            %
            % We installed a factory that creates a subclass of the outer class and overrides ice_postUnmarshal.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            cb = CB2();
            cb.c1seq = {};
            for i = 1:10
                cb.c1seq{i} = C1(i);
            end
            out.writeValue(cb);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            h = IceInternal.ValueHolder();
            is.readValue(@(v) h.set(v), symbol('CB2'));
            is.readPendingValues();
            % At this point, h.value should hold the instance, but ice_postUnmarshal should not have been called yet.
            assert(~isempty(h.value));
            assert(~h.value.postUnmarshalInvoked);
            % Prior to conversion, the member is a CellArrayHandle
            assert(isa(h.value.c1seq, 'IceInternal.CellArrayHandle'));
            % Ending the encapsulation should trigger the conversion and the ice_postUnmarshal callback
            is.endEncapsulation();
            assert(h.value.postUnmarshalInvoked);
            % Now the member should be a cell arary
            assert(isa(h.value.c1seq, 'cell'));
            assert(length(h.value.c1seq) == length(cb.c1seq));
            for i = 1:10
                assert(h.value.c1seq{i}.i == i);
            end

            %
            % Test: class containing sequence of struct containing class. The ice_postUnmarshal callback should
            % be postponed until after the class has converted its member.
            %
            % We installed a factory that creates a subclass of the outer class and overrides ice_postUnmarshal.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            cb = CB3();
            cb.s1seq = S1();
            for i = 1:10
                cb.s1seq(i) = S1(C1(i));
            end
            out.writeValue(cb);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            h = IceInternal.ValueHolder();
            is.readValue(@(v) h.set(v), symbol('CB3'));
            is.readPendingValues();
            % At this point, h.value should hold the instance, but ice_postUnmarshal should not have been called yet.
            assert(~isempty(h.value));
            assert(~h.value.postUnmarshalInvoked);
            % The sequence should be an array of S1
            assert(isa(h.value.s1seq, symbol('S1')));
            % Ending the encapsulation should trigger the conversion and the ice_postUnmarshal callback
            is.endEncapsulation();
            assert(h.value.postUnmarshalInvoked);
            assert(length(h.value.s1seq) == length(cb.s1seq));
            for i = 1:10
                assert(h.value.s1seq(i).c1.i == i);
            end

            %
            % Test: class containing dictionary containing class. The ice_postUnmarshal callback should
            % be postponed until after the class has converted its member.
            %
            % We installed a factory that creates a subclass of the outer class and overrides ice_postUnmarshal.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            cb = CB4();
            for i = 1:10
                cb.c1dict(i) = C1(i);
            end
            out.writeValue(cb);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            h = IceInternal.ValueHolder();
            is.readValue(@(v) h.set(v), symbol('CB4'));
            is.readPendingValues();
            % At this point, h.value should hold the instance, but ice_postUnmarshal should not have been called yet.
            assert(~isempty(h.value));
            assert(~h.value.postUnmarshalInvoked);
            % The member should be a map
            assert(isa(h.value.c1dict, 'containers.Map'));
            % The entry values haven't been converted yet
            assert(isa(h.value.c1dict(1), 'IceInternal.ValueHolder'));
            % Ending the encapsulation should trigger the conversion and the ice_postUnmarshal callback
            is.endEncapsulation();
            assert(h.value.postUnmarshalInvoked);
            assert(length(h.value.c1dict) == length(cb.c1dict));
            assert(isa(h.value.c1dict(1), symbol('C1')));
            for i = 1:10
                assert(h.value.c1dict(i).i == i);
            end

            %
            % Test: class containing dictionary containing struct containing class. The ice_postUnmarshal callback
            % should be postponed until after the class has converted its member.
            %
            % We installed a factory that creates a subclass of the outer class and overrides ice_postUnmarshal.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            cb = CB5();
            for i = 1:10
                cb.s1dict(i) = S1(C1(i));
            end
            out.writeValue(cb);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            h = IceInternal.ValueHolder();
            is.readValue(@(v) h.set(v), symbol('CB5'));
            is.readPendingValues();
            % At this point, h.value should hold the instance, but ice_postUnmarshal should not have been called yet.
            assert(~isempty(h.value));
            assert(~h.value.postUnmarshalInvoked);
            % The member should be a map
            assert(isa(h.value.s1dict, 'containers.Map'));
            % The entry values haven't been converted yet
            assert(isa(h.value.s1dict(1).c1, 'IceInternal.ValueHolder'));
            % Ending the encapsulation should trigger the conversion and the ice_postUnmarshal callback
            is.endEncapsulation();
            assert(h.value.postUnmarshalInvoked);
            assert(length(h.value.s1dict) == length(cb.s1dict));
            assert(isa(h.value.s1dict(1).c1, symbol('C1')));
            for i = 1:10
                assert(h.value.s1dict(i).c1.i == i);
            end

            %
            % Test: class containing sequence of sequence of class. The ice_postUnmarshal callback
            % should be postponed until after the class has converted its member.
            %
            % We installed a factory that creates a subclass of the outer class and overrides ice_postUnmarshal.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            cb = CB6();
            for i = 1:10
                for j = 1:5
                    cb.c1seqseq{i}{j} = C1(i * 10 + j);
                end
            end
            out.writeValue(cb);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            h = IceInternal.ValueHolder();
            is.readValue(@(v) h.set(v), symbol('CB6'));
            is.readPendingValues();
            % At this point, h.value should hold the instance, but ice_postUnmarshal should not have been called yet.
            assert(~isempty(h.value));
            assert(~h.value.postUnmarshalInvoked);
            % The member should be a cell array
            assert(isa(h.value.c1seqseq, 'cell'));
            % The entry values haven't been converted yet
            assert(isa(h.value.c1seqseq{1}, 'IceInternal.CellArrayHandle'));
            % Ending the encapsulation should trigger the conversion and the ice_postUnmarshal callback
            is.endEncapsulation();
            assert(h.value.postUnmarshalInvoked);
            assert(isa(h.value.c1seqseq{1}, 'cell'));
            assert(length(h.value.c1seqseq) == length(cb.c1seqseq));
            for i = 1:10
                assert(length(h.value.c1seqseq{i}) == 5);
                for j = 1:5
                    assert(h.value.c1seqseq{i}{j}.i == i * 10 + j);
                end
            end

            %
            % Test: class containing sequence of sequence of struct containing class. The ice_postUnmarshal callback
            % should be postponed until after the class has converted its member.
            %
            % We installed a factory that creates a subclass of the outer class and overrides ice_postUnmarshal.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            cb = CB7();
            for i = 1:10
                cb.s1seqseq{i} = S1();
                for j = 1:5
                    cb.s1seqseq{i}(j) = S1(C1(i * 10 + j));
                end
            end
            out.writeValue(cb);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            h = IceInternal.ValueHolder();
            is.readValue(@(v) h.set(v), symbol('CB7'));
            is.readPendingValues();
            % At this point, h.value should hold the instance, but ice_postUnmarshal should not have been called yet.
            assert(~isempty(h.value));
            assert(~h.value.postUnmarshalInvoked);
            % The member should be a cell array
            assert(isa(h.value.s1seqseq, 'cell'));
            assert(isa(h.value.s1seqseq{1}, symbol('S1')));
            % Ending the encapsulation should trigger the conversion and the ice_postUnmarshal callback
            is.endEncapsulation();
            assert(h.value.postUnmarshalInvoked);
            assert(isa(h.value.s1seqseq{1}(1).c1, symbol('C1')));
            assert(length(h.value.s1seqseq) == length(cb.s1seqseq));
            for i = 1:10
                assert(length(h.value.s1seqseq{i}) == 5);
                for j = 1:5
                    assert(h.value.s1seqseq{i}(j).c1.i == i * 10 + j);
                end
            end

            %
            % Test: class containing multiple members requiring conversion. The ice_postUnmarshal callback
            % should be postponed until after the class has converted its member.
            %
            % We installed a factory that creates a subclass of the outer class and overrides ice_postUnmarshal.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            cb = CB8();
            cb.s1 = S1(C1(3));
            for i = 1:10
                cb.c1seq{i} = C1(i);
                cb.s1dict(i) = S1(C1(i));
            end
            out.writeValue(cb);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            h = IceInternal.ValueHolder();
            is.readValue(@(v) h.set(v), symbol('CB8'));
            is.readPendingValues();
            % At this point, h.value should hold the instance, but ice_postUnmarshal should not have been called yet.
            assert(~isempty(h.value));
            assert(~h.value.postUnmarshalInvoked);
            assert(isa(h.value.s1.c1, 'IceInternal.ValueHolder'));
            assert(isa(h.value.c1seq, 'IceInternal.CellArrayHandle'));
            assert(isa(h.value.s1dict(1).c1, 'IceInternal.ValueHolder'));
            % Ending the encapsulation should trigger the conversion and the ice_postUnmarshal callback
            is.endEncapsulation();
            assert(h.value.postUnmarshalInvoked);
            assert(isa(h.value.s1.c1, symbol('C1')));
            assert(isa(h.value.c1seq, 'cell'));
            assert(isa(h.value.s1dict(1).c1, symbol('C1')));
            assert(length(h.value.c1seq) == length(cb.c1seq));
            assert(length(h.value.s1dict) == length(cb.s1dict));
            for i = 1:10
                assert(h.value.c1seq{i}.i == i);
                assert(h.value.s1dict(i).c1.i == i);
            end

            %
            % Test: dictionary with struct key and class value type. The dictionary maps to a struct array and
            % temporarily stores IceInternal.ValueHolder objects as its values until converted.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            d = struct.empty();
            for i = 1:10
                d(i).key = StructKey(i, i);
                d(i).value = C1(i);
            end
            StructDict1.write(out, d);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = StructDict1.read(is);
            is.readPendingValues();
            is.endEncapsulation();
            % The result should be a struct array
            assert(isa(r, 'struct'));
            assert(length(r) == length(d));
            % The entry values haven't been converted yet
            assert(isa(r(1).value, 'IceInternal.ValueHolder'));
            r = StructDict1.convert(r);
            assert(isa(r(1).value, symbol('C1')));
            for i = 1:10
                assert(r(i).value.i == i);
            end

            %
            % Test: dictionary with struct key and struct value type containig class. The dictionary maps to a
            % struct array and temporarily stores IceInternal.ValueHolder objects as its values until converted.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            d = struct.empty();
            for i = 1:10
                d(i).key = StructKey(i, i);
                d(i).value = S1(C1(i));
            end
            StructDict2.write(out, d);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = StructDict2.read(is);
            is.readPendingValues();
            is.endEncapsulation();
            % The result should be a struct array
            assert(isa(r, 'struct'));
            assert(length(r) == length(d));
            % The entry values haven't been converted yet
            assert(isa(r(1).value.c1, 'IceInternal.ValueHolder'));
            r = StructDict2.convert(r);
            assert(isa(r(1).value.c1, symbol('C1')));
            for i = 1:10
                assert(r(i).value.c1.i == i);
            end

            %
            % Test: dictionary of dictionary containing class. The inner map temporarily stores IceInternal.ValueHolder
            % objects as its values until converted.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            d = containers.Map('KeyType', 'int32', 'ValueType', 'any');
            for i = 1:10
                inner = containers.Map('KeyType', 'int32', 'ValueType', 'any');
                d(i) = inner;
                for j = 1:5
                    inner(j) = C1(i);
                end
            end
            C1DictDict.write(out, d);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = C1DictDict.read(is);
            is.readPendingValues();
            is.endEncapsulation();
            % The result should be a map
            assert(isa(r, 'containers.Map'));
            assert(length(r) == length(d));
            % The entry values haven't been converted yet
            tmp = r(1);
            assert(isa(tmp(1), 'IceInternal.ValueHolder'));
            r = C1DictDict.convert(r);
            tmp = r(1);
            assert(isa(tmp(1), symbol('C1')));
            assert(length(r) == length(d));
            for i = 1:10
                inner = r(i);
                for j = 1:5
                    assert(inner(j).i == i);
                end
            end

            %
            % Test: dictionary of dictionary containing struct containing class. The struct temporarily stores
            % IceInternal.ValueHolder objects until converted.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            d = containers.Map('KeyType', 'int32', 'ValueType', 'any');
            for i = 1:10
                inner = containers.Map('KeyType', 'int32', 'ValueType', 'any');
                d(i) = inner;
                for j = 1:5
                    inner(j) = S1(C1(i));
                end
            end
            S1DictDict.write(out, d);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            r = S1DictDict.read(is);
            is.readPendingValues();
            is.endEncapsulation();
            % The result should be a map
            assert(isa(r, 'containers.Map'));
            assert(length(r) == length(d));
            % The entry values haven't been converted yet
            tmp = r(1);
            assert(isa(tmp(1).c1, 'IceInternal.ValueHolder'));
            r = S1DictDict.convert(r);
            tmp = r(1);
            assert(isa(tmp(1).c1, symbol('C1')));
            assert(length(r) == length(d));
            for i = 1:10
                inner = r(i);
                for j = 1:5
                    assert(inner(j).c1.i == i);
                end
            end

            %
            % Test: class containing multiple optional members requiring conversion.
            %

            out = communicator.createOutputStream(encoding);
            out.startEncapsulation(format);
            cb = Opt();
            out.writeValue(cb);
            out.writePendingValues();
            out.endEncapsulation();

            is = out.createInputStream();
            is.startEncapsulation();
            h = IceInternal.ValueHolder();
            is.readValue(@(v) h.set(v), symbol('Opt'));
            is.readPendingValues();
            is.endEncapsulation();
            assert(h.value.s1 == Ice.Unset);
            assert(h.value.c1seq == Ice.Unset);
            assert(h.value.s1dict == Ice.Unset);

            %
            % Test: class containing multiple optional members requiring conversion.
            %

            if encoding == Ice.EncodingVersion(1, 1)
                out = communicator.createOutputStream(encoding);
                out.startEncapsulation(format);
                cb = Opt();
                cb.s1 = S1(C1(3));
                cb.c1seq = {};
                cb.s1dict = containers.Map('KeyType', 'int32', 'ValueType', 'any');
                for i = 1:10
                    cb.c1seq{i} = C1(i);
                    cb.s1dict(i) = S1(C1(i));
                end
                out.writeValue(cb);
                out.writePendingValues();
                out.endEncapsulation();

                is = out.createInputStream();
                is.startEncapsulation();
                h = IceInternal.ValueHolder();
                is.readValue(@(v) h.set(v), symbol('Opt'));
                is.readPendingValues();
                is.endEncapsulation();
                assert(isa(h.value.s1.c1, symbol('C1')));
                assert(isa(h.value.c1seq, 'cell'));
                assert(isa(h.value.s1dict(1).c1, symbol('C1')));
                assert(length(h.value.c1seq) == length(cb.c1seq));
                assert(length(h.value.s1dict) == length(cb.s1dict));
                for i = 1:10
                    assert(h.value.c1seq{i}.i == i);
                    assert(h.value.s1dict(i).c1.i == i);
                end
            end

            fprintf('ok\n');
        end
    end
end

function r = symbol(name)
    r = strcat('LocalTest.', name);
end
