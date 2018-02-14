%{
**********************************************************************

Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef AllTests
    methods(Static)
        function r = allTests(app)
            import Test.*;

            communicator = app.communicator();

            fprintf('testing Ice.Admin.Facets property... ');
            assert(length(communicator.getProperties().getPropertyAsList('Ice.Admin.Facets')) == 0);
            communicator.getProperties().setProperty('Ice.Admin.Facets', 'foobar');
            facetFilter = communicator.getProperties().getPropertyAsList('Ice.Admin.Facets');
            assert(length(facetFilter) == 1 && strcmp(facetFilter{1}, 'foobar'));
            communicator.getProperties().setProperty('Ice.Admin.Facets', 'foo\''bar');
            facetFilter = communicator.getProperties().getPropertyAsList('Ice.Admin.Facets');
            assert(length(facetFilter) == 1 && strcmp(facetFilter{1}, 'foo''bar'));
            communicator.getProperties().setProperty('Ice.Admin.Facets', '''foo bar'' toto ''titi''');
            facetFilter = communicator.getProperties().getPropertyAsList('Ice.Admin.Facets');
            assert(length(facetFilter) == 3 && strcmp(facetFilter{1}, 'foo bar') && ...
                   strcmp(facetFilter{2}, 'toto') && strcmp(facetFilter{3}, 'titi'));
            communicator.getProperties().setProperty('Ice.Admin.Facets', '''foo bar\'' toto'' ''titi''');
            facetFilter = communicator.getProperties().getPropertyAsList('Ice.Admin.Facets');
            assert(length(facetFilter) == 2 && strcmp(facetFilter{1}, 'foo bar'' toto') && ...
                   strcmp(facetFilter{2}, 'titi'));
            % communicator.getProperties().setProperty('Ice.Admin.Facets', '''foo bar'' ''toto titi');
            % facetFilter = communicator.getProperties().getPropertyAsList('Ice.Admin.Facets');
            % assert(length(facetFilter) == 0);
            communicator.getProperties().setProperty('Ice.Admin.Facets', '');
            fprintf('ok\n');

            fprintf('testing stringToProxy... ');
            ref = ['d:', app.getTestEndpoint(0)];
            db = communicator.stringToProxy(ref);
            assert(~isempty(db));
            fprintf('ok\n');

            fprintf('testing unchecked cast... ');
            prx = Ice.ObjectPrx.uncheckedCast(db);
            assert(length(prx.ice_getFacet()) == 0);
            prx = Ice.ObjectPrx.uncheckedCast(db, 'facetABCD');
            assert(strcmp(prx.ice_getFacet(), 'facetABCD'));
            prx2 = Ice.ObjectPrx.uncheckedCast(prx);
            assert(strcmp(prx2.ice_getFacet(), 'facetABCD'));
            prx3 = Ice.ObjectPrx.uncheckedCast(prx, '');
            assert(length(prx3.ice_getFacet()) == 0);
            d = DPrx.uncheckedCast(db);
            assert(length(d.ice_getFacet()) == 0);
            df = DPrx.uncheckedCast(db, 'facetABCD');
            assert(strcmp(df.ice_getFacet(), 'facetABCD'));
            df2 = DPrx.uncheckedCast(df);
            assert(strcmp(df2.ice_getFacet(), 'facetABCD'));
            df3 = DPrx.uncheckedCast(df, '');
            assert(length(df3.ice_getFacet()) == 0);
            fprintf('ok\n');

            fprintf('testing checked cast... ');
            prx = Ice.ObjectPrx.checkedCast(db);
            assert(length(prx.ice_getFacet()) == 0);
            prx = Ice.ObjectPrx.checkedCast(db, 'facetABCD');
            assert(strcmp(prx.ice_getFacet(), 'facetABCD'));
            prx2 = Ice.ObjectPrx.checkedCast(prx);
            assert(strcmp(prx2.ice_getFacet(), 'facetABCD'));
            prx3 = Ice.ObjectPrx.checkedCast(prx, '');
            assert(length(prx3.ice_getFacet()) == 0);
            d = DPrx.checkedCast(db);
            assert(length(d.ice_getFacet()) == 0);
            df = DPrx.checkedCast(db, 'facetABCD');
            assert(strcmp(df.ice_getFacet(), 'facetABCD'));
            df2 = DPrx.checkedCast(df);
            assert(strcmp(df2.ice_getFacet(), 'facetABCD'));
            df3 = DPrx.checkedCast(df, '');
            assert(length(df3.ice_getFacet()) == 0);
            fprintf('ok\n');

            fprintf('testing non-facets A, B, C, and D... ');
            d = DPrx.checkedCast(db);
            assert(~isempty(d));
            assert(d == db);
            assert(strcmp(d.callA(), 'A'));
            assert(strcmp(d.callB(), 'B'));
            assert(strcmp(d.callC(), 'C'));
            assert(strcmp(d.callD(), 'D'));
            fprintf('ok\n');

            fprintf('testing facets A, B, C, and D... ');
            df = DPrx.checkedCast(d, 'facetABCD');
            assert(~isempty(df));
            assert(strcmp(df.callA(), 'A'));
            assert(strcmp(df.callB(), 'B'));
            assert(strcmp(df.callC(), 'C'));
            assert(strcmp(df.callD(), 'D'));
            fprintf('ok\n');

            fprintf('testing facets E and F... ');
            ff = FPrx.checkedCast(d, 'facetEF');
            assert(~isempty(ff));
            assert(strcmp(ff.callE(), 'E'));
            assert(strcmp(ff.callF(), 'F'));
            fprintf('ok\n');

            fprintf('testing facet G... ');
            gf = GPrx.checkedCast(ff, 'facetGH');
            assert(~isempty(gf));
            assert(strcmp(gf.callG(), 'G'));
            fprintf('ok\n');

            fprintf('testing whether casting preserves the facet... ');
            hf = HPrx.checkedCast(gf);
            assert(~isempty(hf));
            assert(strcmp(hf.callG(), 'G'));
            assert(strcmp(hf.callH(), 'H'));
            fprintf('ok\n');

            r = gf;
        end
    end
end
