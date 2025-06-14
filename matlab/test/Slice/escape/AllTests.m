% Copyright (c) ZeroC, Inc.

classdef AllTests
    methods(Static)
        function allTests(helper)
            communicator = helper.communicator();

            fprintf('testing enum... ');

            members = enumeration('classdef_.persistent_');
            for i = 0:int32(classdef_.persistent_.LAST) - 1
                % Every enumerator should be escaped and therefore have a trailing underscore.
                name = char(members(i + 1));
                assert(strcmp(name(length(name)), '_'));
                % Ensure ice_getValue is generated correctly.
                assert(members(i + 1) == classdef_.persistent_.ice_getValue(i));
            end

            fprintf('ok\n');

            fprintf('testing struct... ');

            s = classdef_.global_();
            assert(s.case_ == classdef_.persistent_.catch_);
            assert(s.continue_ == 1);
            assert(s.eq_ == 2);
            % Exercise the marshaling code.
            os = Ice.OutputStream(communicator.getEncoding());
            classdef_.global_.ice_write(os, s);
            is = Ice.InputStream(communicator, os.getEncoding(), os.finished());
            s2 = classdef_.global_.ice_read(is);
            assert(isequal(s, s2));

            fprintf('ok\n');

            fprintf('testing class... ');

            c = classdef_.logical_();
            defaultFor = classdef_.global_();
            assert(c.else_ == classdef_.persistent_.break_);
            assert(isempty(c.for_));
            assert(c.int64 == true);
            % Even though c.for_ is empty, we can still marshal it.
            % Exercise the marshaling code.
            os = Ice.OutputStream(communicator.getEncoding());
            os.writeValue(c);
            os.writePendingValues();
            is = Ice.InputStream(communicator, os.getEncoding(), os.finished());
            v = IceInternal.ValueHolder();
            is.readValue(@v.set, 'classdef_.logical_');
            is.readPendingValues();
            assert(v.value.else_ == c.else_);
            assert(v.value.for_.case_ == defaultFor.case_);
            assert(v.value.for_.continue_ == defaultFor.continue_);
            assert(v.value.for_.eq_ == defaultFor.eq_);
            assert(v.value.int64 == c.int64);

            d = classdef_.escaped_xor();
            assert(d.else_ == classdef_.persistent_.break_);
            assert(isempty(d.for_));
            assert(d.int64 == true);
            assert(d.return_ == 1);
            % Even though d.for_ is empty, we can still marshal it.
            % Exercise the marshaling code.
            os = Ice.OutputStream(communicator.getEncoding());
            os.writeValue(d);
            os.writePendingValues();
            is = Ice.InputStream(communicator, os.getEncoding(), os.finished());
            v = IceInternal.ValueHolder();
            is.readValue(@v.set, 'classdef_.escaped_xor');
            is.readPendingValues();
            assert(v.value.else_ == d.else_);
            assert(v.value.for_.case_ == defaultFor.case_);
            assert(v.value.for_.continue_ == defaultFor.continue_);
            assert(v.value.for_.eq_ == defaultFor.eq_);
            assert(v.value.int64 == d.int64);
            assert(v.value.return_ == d.return_);

            p = classdef_.Derived();
            assert(p.while_ == 1);
            assert(p.if_ == 2);
            assert(isempty(p.spmd_));
            assert(p.otherwise_.numEntries == 0);
            % Exercise the marshaling code.
            os = Ice.OutputStream(communicator.getEncoding());
            os.writeValue(p);
            os.writePendingValues();
            is = Ice.InputStream(communicator, os.getEncoding(), os.finished());
            v = IceInternal.ValueHolder();
            is.readValue(@v.set, 'classdef_.Derived');
            is.readPendingValues();
            assert(v.value.while_ == p.while_);
            assert(v.value.if_ == p.if_);

            fprintf('ok\n');

            fprintf('testing exception... ');

            e = classdef_.bitand_();
            assert(strcmp(e.identifier_, '1'));
            assert(strcmp(e.message_, '2'));
            assert(isempty(e.end_));

            g = classdef_.escaped_bitor();
            assert(strcmp(g.identifier_, '1'));
            assert(strcmp(g.message_, '2'));
            assert(isempty(g.end_));
            assert(g.enumeration_ == 1);

            fprintf('ok\n');

            fprintf('testing interface... ');

            assert(exist('classdef_.MyInterfacePrx', 'class') ~= 0);
            m = methods('classdef_.MyInterfacePrx');
            assert(ismember('foobar', m));
            assert(ismember('foobarAsync', m));
            assert(ismember('func', m));
            assert(ismember('funcAsync', m));

            fprintf('ok\n');

            fprintf('testing constant... ');

            assert(classdef_.methods_.value == 1);

            fprintf('ok\n');
        end
    end
end
