// Copyright (c) ZeroC, Inc.

import { TestHelper, test } from "../../Common/TestHelper.js";
import { Outer as RelativeOuter } from "./relative/Test.js";
import { Outer as ModuleOuter } from "test_nested_modules";
import { External } from "./external/Test.js";

export class Client extends TestHelper {
    run() {
        const out = this.getWriter();

        out.writeLine("testing module aggregation with relative includes (no js:module):");

        out.write("testing modules from multiple files are merged correctly... ");

        // Test that types from First.ice are accessible through Outer.Inner
        // This verifies that the nested module merge preserves First from First.ice
        const first = new RelativeOuter.Inner.First(42);
        test(first.value === 42);

        // Test that types from Second.ice are accessible through Outer.Inner
        // This verifies that the nested module merge preserves Second from Second.ice
        const second = new RelativeOuter.Inner.Second("hello");
        test(second.name === "hello");

        // Test that types from Test.ice (which includes both) are accessible
        // This verifies that Combined can use both First and Second
        const combined = new RelativeOuter.Inner.Combined(first, second);
        test(combined.first.value === 42);
        test(combined.second.name === "hello");

        out.writeLine("ok");

        out.write("testing deep modules from multiple files are merged correctly... ");

        // Test that deep nested types from First.ice are accessible through Outer.Inner.Deep
        // This verifies that multi-level module aggregation works
        const deepFirst = new RelativeOuter.Inner.Deep.DeepFirst(100);
        test(deepFirst.deepValue === 100);

        // Test that deep nested types from Second.ice are accessible through Outer.Inner.Deep
        const deepSecond = new RelativeOuter.Inner.Deep.DeepSecond("deep");
        test(deepSecond.deepName === "deep");

        // Test that DeepCombined from Test.ice can use both DeepFirst and DeepSecond
        const deepCombined = new RelativeOuter.Inner.Deep.DeepCombined(deepFirst, deepSecond);
        test(deepCombined.deepFirst.deepValue === 100);
        test(deepCombined.deepSecond.deepName === "deep");

        out.writeLine("ok");

        out.write("testing transitive includes are merged correctly... ");

        // Test that types from FirstDeep.ice (transitively included via First.ice) are accessible
        // through Outer.Inner.Transitive. This verifies that transitive includes are properly
        // aggregated to their direct include parent.
        const transitiveFirst = new RelativeOuter.Inner.Transitive.TransitiveFirst(999);
        test(transitiveFirst.transitiveValue === 999);

        out.writeLine("ok");

        out.write("testing mixed direct + transitive overlap... ");

        // Test that Third (from Third.ice - direct include) is accessible
        const third = new RelativeOuter.Inner.Third(33);
        test(third.thirdValue === 33);

        // Test that DeepThird (from ThirdDeep.ice - transitive include) is accessible.
        // Third.ice defines Outer.Inner.Third but does NOT define Outer.Inner.Deep.
        // Only ThirdDeep.ice (transitive) defines Outer.Inner.Deep.DeepThird.
        // This confirms aggregation doesn't drop deeper modules when only transitive defines them.
        const deepThird = new RelativeOuter.Inner.Deep.DeepThird(333);
        test(deepThird.deepThirdValue === 333);

        out.writeLine("ok");

        out.write("testing forward declared types with js:defined-in... ");

        // Test that ForwardDeclared (defined in Forward.ice, forward declared in Test.ice)
        // is accessible and not double-exported. The js:defined-in metadata tells the compiler
        // to import the type from Forward.ice instead of re-exporting it.
        const forward = new RelativeOuter.Inner.ForwardDeclared("forward-test");
        test(forward.forwardValue === "forward-test");

        // Test that UsesForward can use the forward declared type
        const usesForward = new RelativeOuter.Inner.UsesForward(forward);
        test(usesForward.forward !== null && usesForward.forward.forwardValue === "forward-test");

        out.writeLine("ok");

        out.writeLine("");
        out.writeLine("testing module aggregation with js:module (all files in same module):");

        out.write("testing modules from multiple files are merged correctly... ");

        // Same tests but with js:module - JavaScript should still aggregate,
        // TypeScript should not need inter-file imports since all in same module
        const moduleFirst = new ModuleOuter.Inner.First(42);
        test(moduleFirst.value === 42);

        const moduleSecond = new ModuleOuter.Inner.Second("hello");
        test(moduleSecond.name === "hello");

        const moduleCombined = new ModuleOuter.Inner.Combined(moduleFirst, moduleSecond);
        test(moduleCombined.first.value === 42);
        test(moduleCombined.second.name === "hello");

        out.writeLine("ok");

        out.write("testing deep modules from multiple files are merged correctly... ");

        const moduleDeepFirst = new ModuleOuter.Inner.Deep.DeepFirst(100);
        test(moduleDeepFirst.deepValue === 100);

        const moduleDeepSecond = new ModuleOuter.Inner.Deep.DeepSecond("deep");
        test(moduleDeepSecond.deepName === "deep");

        const moduleDeepCombined = new ModuleOuter.Inner.Deep.DeepCombined(moduleDeepFirst, moduleDeepSecond);
        test(moduleDeepCombined.deepFirst.deepValue === 100);
        test(moduleDeepCombined.deepSecond.deepName === "deep");

        out.writeLine("ok");

        out.write("testing transitive includes are merged correctly... ");

        const moduleTransitiveFirst = new ModuleOuter.Inner.Transitive.TransitiveFirst(999);
        test(moduleTransitiveFirst.transitiveValue === 999);

        out.writeLine("ok");

        out.write("testing mixed direct + transitive overlap... ");

        // Test that Third (from Third.ice - direct include) is accessible
        const moduleThird = new ModuleOuter.Inner.Third(33);
        test(moduleThird.thirdValue === 33);

        // Test that DeepThird (from ThirdDeep.ice - transitive include) is accessible.
        // Third.ice defines Outer.Inner.Third but does NOT define Outer.Inner.Deep.
        // Only ThirdDeep.ice (transitive) defines Outer.Inner.Deep.DeepThird.
        // This confirms aggregation doesn't drop deeper modules when only transitive defines them.
        const moduleDeepThird = new ModuleOuter.Inner.Deep.DeepThird(333);
        test(moduleDeepThird.deepThirdValue === 333);

        out.writeLine("ok");

        out.write("testing forward declared types with js:defined-in... ");

        // Test that ForwardDeclared (defined in Forward.ice, forward declared in Test.ice)
        // is accessible and not double-exported.
        const moduleForward = new ModuleOuter.Inner.ForwardDeclared("forward-test");
        test(moduleForward.forwardValue === "forward-test");

        // Test that UsesForward can use the forward declared type
        const moduleUsesForward = new ModuleOuter.Inner.UsesForward(moduleForward);
        test(moduleUsesForward.forward !== null && moduleUsesForward.forward.forwardValue === "forward-test");

        out.writeLine("ok");

        out.writeLine("");
        out.writeLine("testing external js:module (import from npm module, no aggregation):");

        out.write("testing types from external module are imported correctly... ");

        // Test that External.UsesModuleType uses Outer.Inner.First from test_nested_modules.
        // The external/Test.ice includes module/First.ice (which has js:module:test_nested_modules)
        // but external/Test.ice itself has no js:module, so types from the included file
        // should be imported from the npm module, not aggregated.
        const externalFirst = new ModuleOuter.Inner.First(42);
        const usesModule = new External.UsesModuleType(externalFirst);
        test(usesModule.first.value === 42);

        out.writeLine("ok");
    }
}
