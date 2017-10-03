%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef AllTests
    methods(Static)
        function r = allTests(app)
            import test.Ice.checksum.Test.*;

            communicator = app.communicator();

            ref = ['test:', app.getTestEndpoint(0, '')];
            base = communicator.stringToProxy(ref);
            assert(~isempty(base));

            checksum = ChecksumPrx.checkedCast(base);
            assert(~isempty(checksum));

            %
            % Verify that no checksums are present for local types.
            %
            fprintf('testing checksums... ');
            map = SliceChecksums();
            keys = map.keys();
            for i = 1:length(keys)
                key = keys{i};
                assert(isempty(strfind(key, 'Local')));
            end

            %
            % Get server's Slice checksums.
            %
            d = checksum.getSliceChecksums();

            %
            % Compare the checksums. For a type FooN whose name ends in an integer N,
            % we assume that the server's type does not change for N = 1, and does
            % change for N > 1.
            %
            skeys = d.keys();
            for i = 1:length(skeys)
                n = 0;
                key = skeys{i};
                pos = regexp(key, '\d+');
                if ~isempty(pos)
                    n = str2num(key(pos:end));
                end

                assert(map.isKey(key));
                value = map(key);

                if n <= 1
                    assert(strcmp(value, d(key)));
                else
                    assert(~strcmp(value, d(key)));
                end
            end
            fprintf('ok\n');

            r = checksum;
        end
    end
end
