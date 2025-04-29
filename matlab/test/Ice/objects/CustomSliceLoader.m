% Copyright (c) ZeroC, Inc.

classdef CustomSliceLoader < Ice.SliceLoader
    methods
        function r = newInstance(obj, typeId)
            switch typeId
                case '::Test::B'
                    r = BI();
                case '::Test::C'
                    r = CI();
                % case '::Test::D' handled by ClassSliceLoader
                case '::LocalTest::CB1'
                    r = CB1I();
                case '::LocalTest::CB2'
                    r = CB2I();
                case '::LocalTest::CB3'
                    r = CB3I();
                case '::LocalTest::CB4'
                    r = CB4I();
                case '::LocalTest::CB5'
                    r = CB5I();
                case '::LocalTest::CB6'
                    r = CB6I();
                case '::LocalTest::CB7'
                    r = CB7I();
                case '::LocalTest::CB8'
                    r = CB8I();
                otherwise
                    r = []; % not found
            end
        end
    end
end
