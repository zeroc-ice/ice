%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

**********************************************************************
%}

classdef AllTests
    methods(Static)
        function r = allTests(helper)
            import Test.*;

            communicator = helper.communicator();

            fprintf('testing stringToProxy... ');

            %
            % Test nil proxies.
            %
            p = communicator.stringToProxy('');
            assert(isempty(p));
            p = communicator.propertyToProxy('bogus');
            assert(isempty(p));

            ref = ['test:', helper.getTestEndpoint()];
            base = communicator.stringToProxy(ref);
            assert(~isempty(base));

            b1 = communicator.stringToProxy('test');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && isempty(b1.ice_getIdentity().category) && ...
                   isempty(b1.ice_getAdapterId()) && isempty(b1.ice_getFacet()));
            b1 = communicator.stringToProxy('test ');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && isempty(b1.ice_getIdentity().category) && ...
                   isempty(b1.ice_getFacet()));
            b1 = communicator.stringToProxy(' test ');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && isempty(b1.ice_getIdentity().category) && ...
                   isempty(b1.ice_getFacet()));
            b1 = communicator.stringToProxy(' test');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && isempty(b1.ice_getIdentity().category) && ...
                   isempty(b1.ice_getFacet()));
            b1 = communicator.stringToProxy('''test -f facet''');
            assert(strcmp(b1.ice_getIdentity().name, 'test -f facet') && isempty(b1.ice_getIdentity().category) && ...
                   isempty(b1.ice_getFacet()));
            try
                b1 = communicator.stringToProxy('"test -f facet''');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.ProxyParseException'));
            end
            b1 = communicator.stringToProxy('"test -f facet"');
            assert(strcmp(b1.ice_getIdentity().name, 'test -f facet') && isempty(b1.ice_getIdentity().category) && ...
                   isempty(b1.ice_getFacet()));
            b1 = communicator.stringToProxy('"test -f facet@test"');
            assert(strcmp(b1.ice_getIdentity().name, 'test -f facet@test') && ...
                   isempty(b1.ice_getIdentity().category) && isempty(b1.ice_getFacet()));
            b1 = communicator.stringToProxy('"test -f facet@test @test"');
            assert(strcmp(b1.ice_getIdentity().name, 'test -f facet@test @test') && ...
                   isempty(b1.ice_getIdentity().category) && isempty(b1.ice_getFacet()));
            try
                b1 = communicator.stringToProxy('test test');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.ProxyParseException'));
            end
            b1 = communicator.stringToProxy('test\040test');
            assert(strcmp(b1.ice_getIdentity().name, 'test test') && isempty(b1.ice_getIdentity().category));
            try
                b1 = communicator.stringToProxy('test\777');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.IdentityParseException'));
            end
            b1 = communicator.stringToProxy('test\40test');
            assert(strcmp(b1.ice_getIdentity().name, 'test test'));

            % Test some octal and hex corner cases.
            b1 = communicator.stringToProxy('test\4test');
            assert(strcmp(b1.ice_getIdentity().name, sprintf('test\4test')));
            b1 = communicator.stringToProxy('test\04test');
            assert(strcmp(b1.ice_getIdentity().name, sprintf('test\4test')));
            b1 = communicator.stringToProxy('test\004test');
            assert(strcmp(b1.ice_getIdentity().name, sprintf('test\4test')));
            %
            % In other languages this test is written as:
            %
            % b1 = communicator.stringToProxy('test\1114test')
            %
            % But the MATLAB parser consumes all of the octal digits.
            %
            b1 = communicator.stringToProxy('test\111\064test');
            assert(strcmp(b1.ice_getIdentity().name, sprintf('test\111\064test')));

            b1 = communicator.stringToProxy('test\b\f\n\r\t''"\test');
            assert(strcmp(b1.ice_getIdentity().name, sprintf('test\b\f\n\r\t''"\test')) && ...
                   isempty(b1.ice_getIdentity().category));

            b1 = communicator.stringToProxy('category/test');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && ...
                   strcmp(b1.ice_getIdentity().category, 'category') && isempty(b1.ice_getAdapterId()));

            b1 = communicator.stringToProxy('test@adapter');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && isempty(b1.ice_getIdentity().category) && ...
                   strcmp(b1.ice_getAdapterId(), 'adapter'));
            try
                b1 = communicator.stringToProxy('id@adapter test');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.ProxyParseException'));
            end
            b1 = communicator.stringToProxy('category/test@adapter');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && ...
                   strcmp(b1.ice_getIdentity().category, 'category') && strcmp(b1.ice_getAdapterId(), 'adapter'));
            b1 = communicator.stringToProxy('category/test@adapter:tcp');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && ...
                   strcmp(b1.ice_getIdentity().category, 'category') && strcmp(b1.ice_getAdapterId(), 'adapter:tcp'));
            b1 = communicator.stringToProxy('''category 1/test''@adapter');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && ...
                   strcmp(b1.ice_getIdentity().category, 'category 1') && ...
                   strcmp(b1.ice_getAdapterId(), 'adapter'));
            b1 = communicator.stringToProxy('''category/test 1''@adapter');
            assert(strcmp(b1.ice_getIdentity().name, 'test 1') && ...
                   strcmp(b1.ice_getIdentity().category, 'category') && ...
                   strcmp(b1.ice_getAdapterId(), 'adapter'));
            b1 = communicator.stringToProxy('''category/test''@''adapter 1''');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && ...
                   strcmp(b1.ice_getIdentity().category, 'category') && ...
                   strcmp(b1.ice_getAdapterId(), 'adapter 1'));
            b1 = communicator.stringToProxy('"category \/test@foo/test"@adapter');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && ...
                   strcmp(b1.ice_getIdentity().category, 'category /test@foo') && ...
                   strcmp(b1.ice_getAdapterId(), 'adapter'));
            b1 = communicator.stringToProxy('"category \/test@foo/test"@"adapter:tcp"');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && ...
                   strcmp(b1.ice_getIdentity().category, 'category /test@foo') && ...
                   strcmp(b1.ice_getAdapterId(), 'adapter:tcp'));

            b1 = communicator.stringToProxy('id -f facet');
            assert(strcmp(b1.ice_getIdentity().name, 'id') && isempty(b1.ice_getIdentity().category) && ...
                   strcmp(b1.ice_getFacet(), 'facet'));
            b1 = communicator.stringToProxy('id -f ''facet x''');
            assert(strcmp(b1.ice_getIdentity().name, 'id') && isempty(b1.ice_getIdentity().category) && ...
                   strcmp(b1.ice_getFacet(), 'facet x'));
            b1 = communicator.stringToProxy('id -f "facet x"');
            assert(strcmp(b1.ice_getIdentity().name, 'id') && isempty(b1.ice_getIdentity().category) && ...
                   strcmp(b1.ice_getFacet(), 'facet x'));
            try
                b1 = communicator.stringToProxy('id -f "facet x');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.ProxyParseException'));
            end
            try
                b1 = communicator.stringToProxy('id -f ''facet x');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.ProxyParseException'));
            end
            b1 = communicator.stringToProxy('test -f facet:tcp');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && isempty(b1.ice_getIdentity().category) && ...
                   strcmp(b1.ice_getFacet(), 'facet') && isempty(b1.ice_getAdapterId()));
            b1 = communicator.stringToProxy('test -f "facet:tcp"');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && isempty(b1.ice_getIdentity().category) && ...
                   strcmp(b1.ice_getFacet(), 'facet:tcp') && isempty(b1.ice_getAdapterId()));
            b1 = communicator.stringToProxy('test -f facet@test');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && isempty(b1.ice_getIdentity().category) && ...
                   strcmp(b1.ice_getFacet(), 'facet') && strcmp(b1.ice_getAdapterId(), 'test'));
            b1 = communicator.stringToProxy('test -f ''facet@test''');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && isempty(b1.ice_getIdentity().category) && ...
                   strcmp(b1.ice_getFacet(), 'facet@test') && isempty(b1.ice_getAdapterId()));
            b1 = communicator.stringToProxy('test -f ''facet@test''@test');
            assert(strcmp(b1.ice_getIdentity().name, 'test') && isempty(b1.ice_getIdentity().category) && ...
                   strcmp(b1.ice_getFacet(), 'facet@test') && strcmp(b1.ice_getAdapterId(), 'test'));
            try
                b1 = communicator.stringToProxy('test -f facet@test @test');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.ProxyParseException'));
            end
            b1 = communicator.stringToProxy('test');
            assert(b1.ice_isTwoway());
            b1 = communicator.stringToProxy('test -t');
            assert(b1.ice_isTwoway());
            b1 = communicator.stringToProxy('test -o');
            assert(b1.ice_isOneway());
            b1 = communicator.stringToProxy('test -O');
            assert(b1.ice_isBatchOneway());
            b1 = communicator.stringToProxy('test -d');
            assert(b1.ice_isDatagram());
            b1 = communicator.stringToProxy('test -D');
            assert(b1.ice_isBatchDatagram());
            b1 = communicator.stringToProxy('test');
            assert(~b1.ice_isSecure());
            b1 = communicator.stringToProxy('test -s');
            assert(b1.ice_isSecure());

            try
                b1 = communicator.stringToProxy('test:tcp@adapterId');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.EndpointParseException'));
            end
            % This is an unknown endpoint warning, not a parse exception.
            %
            %try
            %   b1 = communicator.stringToProxy('test -f the:facet:tcp');
            %   assert(false);
            %catch ex
            %    assert(isa(ex, 'Ice.EndpointParseException'));
            %end
            try
                b1 = communicator.stringToProxy('test::tcp');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.EndpointParseException'));
            end
            fprintf('ok\n');

            fprintf('testing propertyToProxy... ');
            prop = communicator.getProperties();
            propertyPrefix = 'Foo.Proxy';
            prop.setProperty(propertyPrefix, 'test:default -p 12010');
            b1 = communicator.propertyToProxy(propertyPrefix);
            assert(strcmp(b1.ice_getIdentity().name, 'test') && isempty(b1.ice_getIdentity().category) && ...
                 isempty(b1.ice_getAdapterId()) && isempty(b1.ice_getFacet()));

            property = [propertyPrefix, '.Locator'];
            assert(isempty(b1.ice_getLocator()));
            prop.setProperty(property, 'locator:default -p 10000');
            b1 = communicator.propertyToProxy(propertyPrefix);
            assert(~isempty(b1.ice_getLocator()) && strcmp(b1.ice_getLocator().ice_getIdentity().name, 'locator'));
            prop.setProperty(property, '');

            property = [propertyPrefix, '.LocatorCacheTimeout'];
            assert(b1.ice_getLocatorCacheTimeout() == -1)
            prop.setProperty(property, '1');
            b1 = communicator.propertyToProxy(propertyPrefix);
            assert(b1.ice_getLocatorCacheTimeout() == 1);
            prop.setProperty(property, '');

            % Now retest with an indirect proxy.
            prop.setProperty(propertyPrefix, 'test');
            property = [propertyPrefix, '.Locator'];
            prop.setProperty(property, 'locator:default -p 10000');
            b1 = communicator.propertyToProxy(propertyPrefix);
            assert(~isempty(b1.ice_getLocator()) && strcmp(b1.ice_getLocator().ice_getIdentity().name, 'locator'));
            prop.setProperty(property, '');

            property = [propertyPrefix, '.LocatorCacheTimeout'];
            assert(b1.ice_getLocatorCacheTimeout() == -1);
            prop.setProperty(property, '1');
            b1 = communicator.propertyToProxy(propertyPrefix);
            assert(b1.ice_getLocatorCacheTimeout() == 1);
            prop.setProperty(property, '');

            % This cannot be tested so easily because the property is cached
            % on communicator initialization.
            %
            %prop.setProperty('Ice.Default.LocatorCacheTimeout', '60');
            %b1 = communicator.propertyToProxy(propertyPrefix);
            %assert(b1.ice_getLocatorCacheTimeout() == 60);
            %prop.setProperty('Ice.Default.LocatorCacheTimeout', '');

            prop.setProperty(propertyPrefix, 'test:default -p 12010');

            property = [propertyPrefix, '.Router'];
            assert(isempty(b1.ice_getRouter()));
            prop.setProperty(property, 'router:default -p 10000');
            b1 = communicator.propertyToProxy(propertyPrefix);
            assert(~isempty(b1.ice_getRouter()) && strcmp(b1.ice_getRouter().ice_getIdentity().name, 'router'));
            prop.setProperty(property, '');

            property = [propertyPrefix, '.PreferSecure'];
            assert(~b1.ice_isPreferSecure());
            prop.setProperty(property, '1');
            b1 = communicator.propertyToProxy(propertyPrefix);
            assert(b1.ice_isPreferSecure());
            prop.setProperty(property, '');

            property = [propertyPrefix, '.ConnectionCached'];
            assert(b1.ice_isConnectionCached());
            prop.setProperty(property, '0');
            b1 = communicator.propertyToProxy(propertyPrefix);
            assert(~b1.ice_isConnectionCached());
            prop.setProperty(property, '');

            property = [propertyPrefix, '.InvocationTimeout'];
            assert(b1.ice_getInvocationTimeout() == -1);
            prop.setProperty(property, '1000');
            b1 = communicator.propertyToProxy(propertyPrefix);
            assert(b1.ice_getInvocationTimeout() == 1000);
            prop.setProperty(property, '');

            property = [propertyPrefix, '.EndpointSelection'];
            assert(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);
            prop.setProperty(property, 'Random');
            b1 = communicator.propertyToProxy(propertyPrefix);
            assert(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);
            prop.setProperty(property, 'Ordered');
            b1 = communicator.propertyToProxy(propertyPrefix);
            assert(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
            prop.setProperty(property, '');

            fprintf('ok\n');

            fprintf('testing proxyToProperty... ');

            b1 = communicator.stringToProxy('test');
            b1 = b1.ice_connectionCached(true);
            b1 = b1.ice_preferSecure(false);
            b1 = b1.ice_endpointSelection(Ice.EndpointSelectionType.Ordered);
            b1 = b1.ice_locatorCacheTimeout(100);
            b1 = b1.ice_invocationTimeout(1234);
            b1 = b1.ice_encodingVersion(Ice.EncodingVersion(1, 0));

            router = communicator.stringToProxy('router');
            router = router.ice_connectionCached(true);
            router = router.ice_preferSecure(true);
            router = router.ice_endpointSelection(Ice.EndpointSelectionType.Random);
            router = router.ice_locatorCacheTimeout(200);
            router = router.ice_invocationTimeout(1500);

            locator = communicator.stringToProxy('locator');
            locator = locator.ice_connectionCached(false);
            locator = locator.ice_preferSecure(true);
            locator = locator.ice_endpointSelection(Ice.EndpointSelectionType.Random);
            locator = locator.ice_locatorCacheTimeout(300);
            locator = locator.ice_invocationTimeout(1500);

            locator = locator.ice_router(Ice.RouterPrx.uncheckedCast(router));
            b1 = b1.ice_locator(Ice.LocatorPrx.uncheckedCast(locator));

            proxyProps = communicator.proxyToProperty(b1, 'Test');
            assert(length(proxyProps) == 21);

            assert(strcmp(proxyProps('Test'), 'test -t -e 1.0'));
            %assert(strcmp(proxyProps('Test.CollocationOptimized'), '1'));
            assert(strcmp(proxyProps('Test.ConnectionCached'), '1'));
            assert(strcmp(proxyProps('Test.PreferSecure'), '0'));
            assert(strcmp(proxyProps('Test.EndpointSelection'), 'Ordered'));
            assert(strcmp(proxyProps('Test.LocatorCacheTimeout'), '100'));
            assert(strcmp(proxyProps('Test.InvocationTimeout'), '1234'));

            assert(strcmp(proxyProps('Test.Locator'), ...
                          ['locator -t -e ', Ice.encodingVersionToString(Ice.currentEncoding())]));
            %assert(strcmp(proxyProps('Test.Locator.CollocationOptimized'), '1'));
            assert(strcmp(proxyProps('Test.Locator.ConnectionCached'), '0'));
            assert(strcmp(proxyProps('Test.Locator.PreferSecure'), '1'));
            assert(strcmp(proxyProps('Test.Locator.EndpointSelection'), 'Random'));
            assert(strcmp(proxyProps('Test.Locator.LocatorCacheTimeout'), '300'));
            assert(strcmp(proxyProps('Test.Locator.InvocationTimeout'), '1500'));

            assert(strcmp(proxyProps('Test.Locator.Router'), ...
                          ['router -t -e ', Ice.encodingVersionToString(Ice.currentEncoding())]));
            %assert(strcmp(proxyProps('Test.Locator.Router.CollocationOptimized'), '0'));
            assert(strcmp(proxyProps('Test.Locator.Router.ConnectionCached'), '1'));
            assert(strcmp(proxyProps('Test.Locator.Router.PreferSecure'), '1'));
            assert(strcmp(proxyProps('Test.Locator.Router.EndpointSelection'), 'Random'));
            assert(strcmp(proxyProps('Test.Locator.Router.LocatorCacheTimeout'), '200'));
            assert(strcmp(proxyProps('Test.Locator.Router.InvocationTimeout'), '1500'));

            fprintf('ok\n');

            fprintf('testing ice_getCommunicator... ');

            assert(base.ice_getCommunicator() == communicator);

            fprintf('ok\n');

            fprintf('testing proxy methods... ');

            assert(strcmp(communicator.identityToString(base.ice_identity(Ice.stringToIdentity('other')).ice_getIdentity()), 'other'));

            %{
            % TODO
            %
            % Verify that ToStringMode is passed correctly
            %
            euroStr = sprintf('\x7F\xE2\x82\xAC');
            ident = Ice.Identity('test', euroStr);

            idStr = Ice.identityToString(ident, Ice.ToStringMode.Unicode);
            assert(strcmp(idStr, sprintf('\x007f\xe2\x82\xac/test')));
            ident2 = Ice.stringToIdentity(idStr);
            assert(ident == ident2);
            assert(strcmp(Ice.identityToString(ident), idStr));

            idStr = Ice.identityToString(ident, Ice.ToStringMode.ASCII)
            assert(idStr == "\\u007f\\u20ac/test")
            ident2 = Ice.stringToIdentity(idStr)
            assert(ident == ident2)

            idStr = Ice.identityToString(ident, Ice.ToStringMode.Compat)
            assert(idStr == "\\177\\342\\202\\254/test")
            ident2 = Ice.stringToIdentity(idStr)
            assert(ident == ident2)

            ident2 = Ice.stringToIdentity(communicator.identityToString(ident))
            assert(ident == ident2)
            %}

            assert(strcmp(base.ice_facet('facet').ice_getFacet(), 'facet'));
            assert(strcmp(base.ice_adapterId('id').ice_getAdapterId(), 'id'));
            assert(base.ice_twoway().ice_isTwoway());
            assert(base.ice_oneway().ice_isOneway());
            assert(base.ice_batchOneway().ice_isBatchOneway());
            assert(base.ice_datagram().ice_isDatagram());
            assert(base.ice_batchDatagram().ice_isBatchDatagram());
            assert(base.ice_secure(true).ice_isSecure());
            assert(~base.ice_secure(false).ice_isSecure());
            %assert(base.ice_collocationOptimized(true).ice_isCollocationOptimized());
            %assert(~base.ice_collocationOptimized(false).ice_isCollocationOptimized());
            assert(base.ice_preferSecure(true).ice_isPreferSecure());
            assert(~base.ice_preferSecure(false).ice_isPreferSecure());
            assert(base.ice_encodingVersion(Ice.EncodingVersion(1, 0)).ice_getEncodingVersion() == Ice.EncodingVersion(1, 0));
            assert(base.ice_encodingVersion(Ice.EncodingVersion(1, 1)).ice_getEncodingVersion() == Ice.EncodingVersion(1, 1));
            assert(base.ice_encodingVersion(Ice.EncodingVersion(1, 0)).ice_getEncodingVersion() ~= Ice.EncodingVersion(1, 1));

            try
                base.ice_timeout(0);
                assert(false);
            catch ex
            end

            try
                base.ice_timeout(-1);
            catch ex
                assert(false);
            end

            try
                base.ice_timeout(-2);
                assert(false);
            catch ex
            end

            try
                base.ice_invocationTimeout(0);
                assert(false);
            catch ex
            end

            try
                base.ice_invocationTimeout(-1);
            catch ex
                assert(false);
            end

            try
                base.ice_invocationTimeout(-2);
                assert(false);
            catch ex
            end

            try
                base.ice_locatorCacheTimeout(0);
            catch ex
                assert(false);
            end

            try
                base.ice_locatorCacheTimeout(-1);
            catch ex
                assert(false);
            end

            try
                base.ice_locatorCacheTimeout(-2);
                assert(false);
            catch ex
            end

            fprintf('ok\n');

            fprintf('testing proxy comparison... ');

            assert(communicator.stringToProxy('foo') == communicator.stringToProxy('foo'));
            assert(communicator.stringToProxy('foo') ~= communicator.stringToProxy('foo2'));
            %assert(communicator.stringToProxy('foo') < communicator.stringToProxy('foo2'));
            %assert(~(communicator.stringToProxy('foo2') < communicator.stringToProxy('foo')));

            compObj = communicator.stringToProxy('foo');

            assert(compObj.ice_facet('facet') == compObj.ice_facet('facet'));
            assert(compObj.ice_facet('facet') ~= compObj.ice_facet('facet1'));
            %assert(compObj.ice_facet('facet') < compObj.ice_facet('facet1'));
            %assert(~(compObj.ice_facet('facet') < compObj.ice_facet('facet')));

            assert(compObj.ice_oneway() == compObj.ice_oneway());
            assert(compObj.ice_oneway() ~= compObj.ice_twoway());
            %assert(compObj.ice_twoway() < compObj.ice_oneway());
            %assert(~(compObj.ice_oneway() < compObj.ice_twoway()));

            assert(compObj.ice_secure(true) == compObj.ice_secure(true));
            assert(compObj.ice_secure(false) ~= compObj.ice_secure(true));
            %assert(compObj.ice_secure(false) < compObj.ice_secure(true));
            %assert(~(compObj.ice_secure(true) < compObj.ice_secure(false)));

            %assert(compObj.ice_collocationOptimized(true) == compObj.ice_collocationOptimized(true));
            %assert(compObj.ice_collocationOptimized(false) ~= compObj.ice_collocationOptimized(true));
            %assert(compObj.ice_collocationOptimized(false) < compObj.ice_collocationOptimized(true));
            %assert(~(compObj.ice_collocationOptimized(true) < compObj.ice_collocationOptimized(false)));

            assert(compObj.ice_connectionCached(true) == compObj.ice_connectionCached(true));
            assert(compObj.ice_connectionCached(false) ~= compObj.ice_connectionCached(true));
            %assert(compObj.ice_connectionCached(false) < compObj.ice_connectionCached(true));
            %assert(~(compObj.ice_connectionCached(true) < compObj.ice_connectionCached(false)));

            assert(compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random) == ...
                 compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random));
            assert(compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random) ~= ...
                 compObj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered));
            %assert(compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random) < ...
            %     compObj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered));
            %assert(~(compObj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered) < ...
            %     compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random)));

            assert(compObj.ice_connectionId('id2') == compObj.ice_connectionId('id2'));
            assert(compObj.ice_connectionId('id1') ~= compObj.ice_connectionId('id2'));
            %assert(compObj.ice_connectionId('id1') < compObj.ice_connectionId('id2'));
            %assert(~(compObj.ice_connectionId('id2') < compObj.ice_connectionId('id1')));
            assert(strcmp(compObj.ice_connectionId('id1').ice_getConnectionId(), 'id1'));
            assert(strcmp(compObj.ice_connectionId('id2').ice_getConnectionId(), 'id2'));

            assert(compObj.ice_compress(true) == compObj.ice_compress(true));
            assert(compObj.ice_compress(false) ~= compObj.ice_compress(true));
            %assert(compObj.ice_compress(false) < compObj.ice_compress(true));
            %assert(~(compObj.ice_compress(true) < compObj.ice_compress(false)));

            assert(compObj.ice_getCompress() == Ice.Unset);
            assert(compObj.ice_compress(true).ice_getCompress() == true);
            assert(compObj.ice_compress(false).ice_getCompress() == false);

            assert(compObj.ice_timeout(20) == compObj.ice_timeout(20));
            assert(compObj.ice_timeout(10) ~= compObj.ice_timeout(20));
            %assert(compObj.ice_timeout(10) < compObj.ice_timeout(20));
            %assert(~(compObj.ice_timeout(20) < compObj.ice_timeout(10)));

            assert(compObj.ice_getTimeout() == Ice.Unset);
            assert(compObj.ice_timeout(10).ice_getTimeout() == 10);
            assert(compObj.ice_timeout(20).ice_getTimeout() == 20);

            loc1 = Ice.LocatorPrx.uncheckedCast(communicator.stringToProxy('loc1:default -p 10000'));
            loc2 = Ice.LocatorPrx.uncheckedCast(communicator.stringToProxy('loc2:default -p 10000'));
            assert(compObj.ice_locator([]) == compObj.ice_locator([]));
            assert(compObj.ice_locator(loc1) == compObj.ice_locator(loc1));
            assert(compObj.ice_locator(loc1) ~= compObj.ice_locator([]));
            assert(compObj.ice_locator([]) ~= compObj.ice_locator(loc2));
            assert(compObj.ice_locator(loc1) ~= compObj.ice_locator(loc2));
            %assert(compObj.ice_locator([]) < compObj.ice_locator(loc1));
            %assert(~(compObj.ice_locator(loc1) < compObj.ice_locator([])));
            %assert(compObj.ice_locator(loc1) < compObj.ice_locator(loc2));
            %assert(~(compObj.ice_locator(loc2) < compObj.ice_locator(loc1)));

            rtr1 = Ice.RouterPrx.uncheckedCast(communicator.stringToProxy('rtr1:default -p 10000'));
            rtr2 = Ice.RouterPrx.uncheckedCast(communicator.stringToProxy('rtr2:default -p 10000'));
            assert(compObj.ice_router([]) == compObj.ice_router([]));
            assert(compObj.ice_router(rtr1) == compObj.ice_router(rtr1));
            assert(compObj.ice_router(rtr1) ~= compObj.ice_router([]));
            assert(compObj.ice_router([]) ~= compObj.ice_router(rtr2));
            assert(compObj.ice_router(rtr1) ~= compObj.ice_router(rtr2));
            %assert(compObj.ice_router([]) < compObj.ice_router(rtr1));
            %assert(~(compObj.ice_router(rtr1) < compObj.ice_router([])));
            %assert(compObj.ice_router(rtr1) < compObj.ice_router(rtr2));
            %assert(~(compObj.ice_router(rtr2) < compObj.ice_router(rtr1)));

            ctx1 = containers.Map('KeyType', 'char', 'ValueType', 'char');
            ctx1('ctx1') = 'v1';
            ctx2 = containers.Map('KeyType', 'char', 'ValueType', 'char');
            ctx2('ctx2') = 'v2';
            empty = containers.Map('KeyType', 'char', 'ValueType', 'char');
            assert(compObj.ice_context(empty) == compObj.ice_context(empty));
            assert(compObj.ice_context(ctx1) == compObj.ice_context(ctx1));
            assert(compObj.ice_context(ctx1) ~= compObj.ice_context(empty));
            assert(compObj.ice_context(empty) ~= compObj.ice_context(ctx2));
            assert(compObj.ice_context(ctx1) ~= compObj.ice_context(ctx2));
            %assert(compObj.ice_context(ctx1) < compObj.ice_context(ctx2));
            %assert(~(compObj.ice_context(ctx2) < compObj.ice_context(ctx1)));

            assert(compObj.ice_preferSecure(true) == compObj.ice_preferSecure(true));
            assert(compObj.ice_preferSecure(true) ~= compObj.ice_preferSecure(false));
            %assert(compObj.ice_preferSecure(false) < compObj.ice_preferSecure(true));
            %assert(~(compObj.ice_preferSecure(true) < compObj.ice_preferSecure(false)));

            compObj1 = communicator.stringToProxy('foo:tcp -h 127.0.0.1 -p 10000');
            compObj2 = communicator.stringToProxy('foo:tcp -h 127.0.0.1 -p 10001');
            assert(compObj1 ~= compObj2);
            %assert(compObj1 < compObj2);
            %assert(~(compObj2 < compObj1));

            compObj1 = communicator.stringToProxy('foo@MyAdapter1');
            compObj2 = communicator.stringToProxy('foo@MyAdapter2');
            assert(compObj1 ~= compObj2);
            %assert(compObj1 < compObj2);
            %assert(~(compObj2 < compObj1));

            assert(compObj1.ice_locatorCacheTimeout(20) == compObj1.ice_locatorCacheTimeout(20));
            assert(compObj1.ice_locatorCacheTimeout(10) ~= compObj1.ice_locatorCacheTimeout(20));
            %assert(compObj1.ice_locatorCacheTimeout(10) < compObj1.ice_locatorCacheTimeout(20));
            %assert(~(compObj1.ice_locatorCacheTimeout(20) < compObj1.ice_locatorCacheTimeout(10)));

            assert(compObj1.ice_invocationTimeout(20) == compObj1.ice_invocationTimeout(20));
            assert(compObj1.ice_invocationTimeout(10) ~= compObj1.ice_invocationTimeout(20));
            %assert(compObj1.ice_invocationTimeout(10) < compObj1.ice_invocationTimeout(20));
            %assert(~(compObj1.ice_invocationTimeout(20) < compObj1.ice_invocationTimeout(10)));

            compObj1 = communicator.stringToProxy('foo:tcp -h 127.0.0.1 -p 1000');
            compObj2 = communicator.stringToProxy('foo@MyAdapter1');
            assert(compObj1 ~= compObj2);
            %assert(compObj1 < compObj2);
            %assert(~(compObj2 < compObj1));

            endpts1 = communicator.stringToProxy('foo:tcp -h 127.0.0.1 -p 10000').ice_getEndpoints();
            endpts2 = communicator.stringToProxy('foo:tcp -h 127.0.0.1 -p 10001').ice_getEndpoints();
            assert(endpts1{1} ~= endpts2{1});
            %assert(endpts1 < endpts2);
            %assert(~(endpts2 < endpts1));
            endpts3 = communicator.stringToProxy('foo:tcp -h 127.0.0.1 -p 10000').ice_getEndpoints();
            assert(endpts1{1} == endpts3{1});

            assert(compObj1.ice_encodingVersion(Ice.EncodingVersion(1, 0)) == ...
                   compObj1.ice_encodingVersion(Ice.EncodingVersion(1, 0)));
            assert(compObj1.ice_encodingVersion(Ice.EncodingVersion(1, 0)) ~= ...
                   compObj1.ice_encodingVersion(Ice.EncodingVersion(1, 1)));
            %assert(compObj.ice_encodingVersion(Ice.EncodingVersion(1, 0)) < ...
            %       compObj.ice_encodingVersion(Ice.EncodingVersion(1, 1)));
            %assert(~(compObj.ice_encodingVersion(Ice.EncodingVersion(1, 1)) < ...
            %         compObj.ice_encodingVersion(Ice.EncodingVersion(1, 0))));

            baseConnection = base.ice_getConnection();
            if ~isempty(baseConnection)
                baseConnection2 = base.ice_connectionId('base2').ice_getConnection();
                compObj1 = compObj1.ice_fixed(baseConnection);
                compObj2 = compObj2.ice_fixed(baseConnection2);
                assert(compObj1 ~= compObj2);
            end

            fprintf('ok\n');

            fprintf('testing checked cast... ');
            cl = MyClassPrx.checkedCast(base);
            assert(~isempty(cl));

            derived = MyDerivedClassPrx.checkedCast(cl);
            assert(~isempty(derived));
            assert(cl == base);
            assert(derived == base);
            assert(cl == derived);

            loc = Ice.LocatorPrx.checkedCast(base);
            assert(isempty(loc));

            %
            % Upcasting
            %
            cl2 = MyClassPrx.checkedCast(derived);
            obj = Ice.ObjectPrx.checkedCast(derived);
            assert(~isempty(cl2));
            assert(~isempty(obj));
            assert(cl2 == obj);
            assert(cl2 == derived);

            fprintf('ok\n');

            fprintf('testing checked cast with context... ');
            tccp = MyClassPrx.checkedCast(base);
            c = tccp.getContext();
            assert(isempty(c));

            c = containers.Map('KeyType', 'char', 'ValueType', 'char');
            c('one') = 'hello';
            c('two') = 'world';
            tccp = MyClassPrx.checkedCast(base, c);
            c2 = tccp.getContext();
            assert(isequal(c, c2));
            fprintf('ok\n');

            fprintf('testing ice_fixed... ');
            connection = cl.ice_getConnection();
            if ~isempty(connection)
                prx = cl.ice_fixed(connection); % Test factory method return type
                prx.ice_ping();
                assert(cl.ice_secure(true).ice_fixed(connection).ice_isSecure());
                assert(strcmp(cl.ice_facet('facet').ice_fixed(connection).ice_getFacet(), 'facet'));
                assert(cl.ice_oneway().ice_fixed(connection).ice_isOneway());
                ctx = containers.Map('KeyType', 'char', 'ValueType', 'char');
                ctx('one') = 'hello';
                ctx('two') = 'world';
                assert(isempty(cl.ice_fixed(connection).ice_getContext()));
                assert(cl.ice_context(ctx).ice_fixed(connection).ice_getContext().length() == 2);
                assert(cl.ice_fixed(connection).ice_getInvocationTimeout() == -1);
                assert(cl.ice_invocationTimeout(10).ice_fixed(connection).ice_getInvocationTimeout() == 10);
                assert(cl.ice_fixed(connection).ice_getConnection() == connection);
                assert(cl.ice_fixed(connection).ice_fixed(connection).ice_getConnection() == connection);
                assert(cl.ice_compress(true).ice_fixed(connection).ice_getCompress() == true);
                assert(cl.ice_fixed(connection).ice_getTimeout() == Ice.Unset);
                fixedConnection = cl.ice_connectionId('ice_fixed').ice_getConnection();
                assert(cl.ice_fixed(connection).ice_fixed(fixedConnection).ice_getConnection() == fixedConnection);
                try
                    cl.ice_secure(~connection.getEndpoint().getInfo().secure()).ice_fixed(connection).ice_ping();
                catch ex
                    if ~isa(ex, 'Ice.NoEndpointException')
                        rethrow(ex);
                    end
                end
                try
                    cl.ice_datagram().ice_fixed(connection).ice_ping();
                catch ex
                    if ~isa(ex, 'Ice.NoEndpointException')
                        rethrow(ex);
                    end
                end
            else
                try
                    cl.ice_fixed(connection);
                    assert(false);
                catch ex
                    % Expected with null connection.
                end
            end
            fprintf('ok\n');

            fprintf('testing encoding versioning... ');
            ref20 = 'test -e 2.0:default -p 12010';
            cl20 = MyClassPrx.uncheckedCast(communicator.stringToProxy(ref20));
            try
                cl20.ice_ping();
                assert(false);
            catch ex
                % Server 2.0 endpoint doesn't support 1.1 version.
                assert(isa(ex, 'Ice.UnsupportedEncodingException'));
            end

            ref10 = 'test -e 1.0:default -p 12010';
            cl10 = MyClassPrx.uncheckedCast(communicator.stringToProxy(ref10));
            cl10.ice_ping();
            cl10.ice_encodingVersion(Ice.EncodingVersion(1, 0)).ice_ping();
            cl.ice_encodingVersion(Ice.EncodingVersion(1, 0)).ice_ping();

            % 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
            % call will use the 1.1 encoding
            ref13 = 'test -e 1.3:default -p 12010';
            cl13 = MyClassPrx.uncheckedCast(communicator.stringToProxy(ref13));
            cl13.ice_ping();

            fprintf('ok\n');

            fprintf('testing opaque endpoints... ');

            try
                % Invalid -x option
                p = communicator.stringToProxy('id:opaque -t 99 -v abc -x abc');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.EndpointParseException'));
            end

            try
                % Missing -t and -v
                p = communicator.stringToProxy('id:opaque');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.EndpointParseException'));
            end

            try
                % Repeated -t
                p = communicator.stringToProxy('id:opaque -t 1 -t 1 -v abc');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.EndpointParseException'));
            end

            try
                % Repeated -v
                p = communicator.stringToProxy('id:opaque -t 1 -v abc -v abc')
                assert(false)
            catch ex
                assert(isa(ex, 'Ice.EndpointParseException'));
            end

            try
                % Missing -t
                p = communicator.stringToProxy('id:opaque -v abc');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.EndpointParseException'));
            end

            try
                % Missing -v
                p = communicator.stringToProxy('id:opaque -t 1');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.EndpointParseException'));
            end

            try
                % Missing arg for -t
                p = communicator.stringToProxy('id:opaque -t -v abc');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.EndpointParseException'));
            end

            try
                % Missing arg for -v
                p = communicator.stringToProxy('id:opaque -t 1 -v');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.EndpointParseException'));
            end

            try
                % Not a number for -t
                p = communicator.stringToProxy('id:opaque -t x -v abc');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.EndpointParseException'));
            end

            try
                % < 0 for -t
                p = communicator.stringToProxy('id:opaque -t -1 -v abc');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.EndpointParseException'));
            end

            try
                % Invalid char for -v
                p = communicator.stringToProxy('id:opaque -t 99 -v x?c');
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.EndpointParseException'));
            end

            % Legal TCP endpoint expressed as opaque endpoint
            p1 = communicator.stringToProxy('test -e 1.1:opaque -t 1 -e 1.0 -v CTEyNy4wLjAuMeouAAAQJwAAAA==');
            pstr = communicator.proxyToString(p1);
            assert(strcmp(pstr, 'test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000'));

            % Opaque endpoint encoded with 1.1 encoding.
            p2 = communicator.stringToProxy('test -e 1.1:opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==');
            assert(strcmp(communicator.proxyToString(p2), 'test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000'));

            if communicator.getProperties().getPropertyAsInt('Ice.IPv6') == 0
                % Working?
                ssl = strcmp(communicator.getProperties().getProperty('Ice.Default.Protocol'), 'ssl');
                tcp = strcmp(communicator.getProperties().getProperty('Ice.Default.Protocol'), 'tcp');
                if tcp
                    p1.ice_encodingVersion(Ice.EncodingVersion(1, 0)).ice_ping();
                end

                % Two legal TCP endpoints expressed as opaque endpoints
                p1 = communicator.stringToProxy('test -e 1.0:opaque -t 1 -e 1.0 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:opaque -t 1 -e 1.0 -v CTEyNy4wLjAuMusuAAAQJwAAAA==');
                pstr = communicator.proxyToString(p1);
                assert(strcmp(pstr, 'test -t -e 1.0:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000'));

                %
                % Test that an SSL endpoint and a nonsense endpoint get written
                % back out as an opaque endpoint.
                %
                p1 = communicator.stringToProxy('test -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch');
                pstr = communicator.proxyToString(p1);
                if ssl
                    assert(strcmp(pstr, 'test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch'));
                elseif tcp
                    assert(strcmp(pstr, 'test -t -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch'));
                end

                %
                % Try to invoke on the SSL endpoint to verify that we get a
                % NoEndpointException (or ConnectionRefusedException when
                % running with SSL).
                %
                try
                    p1.ice_encodingVersion(Ice.EncodingVersion(1, 0)).ice_ping();
                    assert(false);
                catch ex
                    if isa(ex, 'Ice.NoEndpointException')
                        assert(~ssl);
                    elseif isa(ex, 'Ice.ConnectFailedException')
                        assert(~tcp);
                    else
                        rethrow(ex);
                    end
                end

                %
                % Test that the proxy with an SSL endpoint and a nonsense
                % endpoint (which the server doesn't understand either) can be
                % sent over the wire and returned by the server without losing
                % the opaque endpoints.
                %
                p2 = derived.echo(p1);
                pstr = communicator.proxyToString(p2);
                if ssl
                    assert(strcmp(pstr, 'test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch'));
                elseif tcp
                    assert(strcmp(pstr, 'test -t -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch'));
                end
            end

            fprintf('ok\n');

            r = cl;
        end
    end
end
