% Copyright (c) ZeroC, Inc.

classdef AllTests
    methods(Static)
        function allTests(helper)
            communicator = helper.communicator();

            fprintf('testing enum... ');

            members = enumeration('escaped_classdef.bitand');
            for i = 0:int32(escaped_classdef.bitand.LAST) - 1
                % Every enumerator should be escaped and therefore have a trailing underscore.
                name = char(members(i + 1));
                assert(strcmp(name(length(name)), '_'));
                % Ensure ice_getValue is generated correctly.
                assert(members(i + 1) == escaped_classdef.bitand.ice_getValue(i));
            end

            fprintf('ok\n');

            fprintf('testing struct... ');

            s = escaped_classdef.bitor();
            assert(s.case_ == escaped_classdef.bitand.catch_);
            assert(s.continue_ == 1);
            assert(s.eq_ == 2);
            % Exercise the marshaling code.
            os = Ice.OutputStream(communicator.getEncoding());
            escaped_classdef.bitor.ice_write(os, s);
            is = Ice.InputStream(communicator, os.getEncoding(), os.finished());
            s2 = escaped_classdef.bitor.ice_read(is);
            assert(isequal(s, s2));

            fprintf('ok\n');

            fprintf('testing class... ');

            c = escaped_classdef.logical();
            assert(c.else_ == escaped_classdef.bitand.break_);
            assert(c.for_.case_ == escaped_classdef.bitand.catch_);
            assert(c.for_.continue_ == 1);
            assert(c.for_.eq_ == 2);
            assert(c.int64 == true);
            % Exercise the marshaling code.
            os = Ice.OutputStream(communicator.getEncoding());
            os.writeValue(c);
            os.writePendingValues();
            is = Ice.InputStream(communicator, os.getEncoding(), os.finished());
            v = IceInternal.ValueHolder();
            is.readValue(@v.set, 'escaped_classdef.logical');
            is.readPendingValues();
            assert(v.value.else_ == c.else_);
            assert(v.value.for_.case_ == c.for_.case_);
            assert(v.value.for_.continue_ == c.for_.continue_);
            assert(v.value.for_.eq_ == c.for_.eq_);
            assert(v.value.int64 == c.int64);

            d = escaped_classdef.xor_();
            assert(d.else_ == escaped_classdef.bitand.break_);
            assert(d.for_.case_ == escaped_classdef.bitand.catch_);
            assert(d.for_.continue_ == 1);
            assert(d.for_.eq_ == 2);
            assert(d.int64 == true);
            assert(d.return_ == 1);
            % Exercise the marshaling code.
            os = Ice.OutputStream(communicator.getEncoding());
            os.writeValue(d);
            os.writePendingValues();
            is = Ice.InputStream(communicator, os.getEncoding(), os.finished());
            v = IceInternal.ValueHolder();
            is.readValue(@v.set, 'escaped_classdef.xor_');
            is.readPendingValues();
            assert(v.value.else_ == d.else_);
            assert(v.value.for_.case_ == d.for_.case_);
            assert(v.value.for_.continue_ == d.for_.continue_);
            assert(v.value.for_.eq_ == d.for_.eq_);
            assert(v.value.int64 == d.int64);
            assert(v.value.return_ == d.return_);

            p = escaped_classdef.properties_();
            assert(p.while_ == 1);
            assert(p.if_ == 2);
            assert(isempty(p.spmd_));
            assert(isempty(p.otherwise_));
            % Exercise the marshaling code.
            os = Ice.OutputStream(communicator.getEncoding());
            os.writeValue(p);
            os.writePendingValues();
            is = Ice.InputStream(communicator, os.getEncoding(), os.finished());
            v = IceInternal.ValueHolder();
            is.readValue(@v.set, 'escaped_classdef.properties_');
            is.readPendingValues();
            assert(v.value.while_ == p.while_);
            assert(v.value.if_ == p.if_);

            fprintf('ok\n');

            fprintf('testing exception... ');

            e = escaped_classdef.persistent_();
            assert(isempty(e.identifier_));
            assert(isempty(e.message_));
            assert(isempty(e.end_));

            g = escaped_classdef.global_();
            assert(isempty(g.identifier_));
            assert(isempty(g.message_));
            assert(isempty(g.end_));
            assert(isempty(g.enumeration_));

            fprintf('ok\n');

            fprintf('testing interface... ');

            assert(exist('escaped_classdef.MyInterfacePrx', 'class') ~= 0);
            m = methods('escaped_classdef.MyInterfacePrx');
            assert(ismember('foobar', m));
            assert(ismember('foobarAsync', m));
            assert(ismember('func', m));
            assert(ismember('funcAsync', m));

            fprintf('ok\n');

            fprintf('testing constant... ');

            assert(escaped_classdef.methods_.value == 1);

            fprintf('ok\n');
        end
    end
end
