// Copyright (c) ZeroC, Inc.

import { TestHelper, test } from "../../Common/TestHelper.js";
import { acme } from "./Test.js";

export class Client extends TestHelper {
    async run() {
        const a = new acme.foo.One("One");
        const b = new acme.foo.baz.quux.One("One");
        const c = new acme.foo.max_.One("One");

        test(a.ice_id() == "::foo::One");
        test(b.ice_id() == "::foo::baz::One");
        test(c.ice_id() == "::foo::max::One");
    }
}
