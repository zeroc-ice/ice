// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test, Inner } from "./Test.js";
import { TestHelper } from "../../Common/TestHelper.js";

const test = TestHelper.test;

export class Client extends TestHelper {
    async allTests() {
        const out = this.getWriter();
        const communicator = this.communicator();

        out.write("test using same type name in different Slice modules... ");

        {
            const i1 = new Test.MyInterfacePrx(communicator, `i1:${this.getTestEndpoint()}`);
            const s1 = new Test.MyStruct(0);

            const [s2, s3] = await i1.opMyStruct(s1);
            test(s2.equals(s1));
            test(s3.equals(s1));

            const [sseq2, sseq3] = await i1.opMyStructSeq([s1]);
            test(sseq2[0].equals(s1));
            test(sseq3[0].equals(s1));

            const smap1 = new Map([["a", s1]]);
            const [smap2, smap3] = await i1.opMyStructMap(smap1);
            test(smap2.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            const c1 = new Test.MyClass(s1);

            const [c2, c3] = await i1.opMyClass(c1);
            test(c2.s.equals(s1));
            test(c3.s.equals(s1));

            const [cseq2, cseq3] = await i1.opMyClassSeq([c1]);
            test(cseq2[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            const cmap1 = new Map([["a", c1]]);
            const [cmap2, cmap3] = await i1.opMyClassMap(cmap1);
            test(cmap2.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));

            const e = await i1.opMyEnum(Test.MyEnum.v1);
            test(e == Test.MyEnum.v1);

            const s = await i1.opMyOtherStruct(new Test.MyOtherStruct("MyOtherStruct"));
            test(s.s == "MyOtherStruct");

            const c = await i1.opMyOtherClass(new Test.MyOtherClass("MyOtherClass"));
            test(c.s == "MyOtherClass");
        }

        {
            const i2 = new Test.Inner.Inner2.MyInterfacePrx(communicator, `i2:${this.getTestEndpoint()}`);
            const s1 = new Test.Inner.Inner2.MyStruct(0);

            const [s2, s3] = await i2.opMyStruct(s1);
            test(s2.equals(s1));
            test(s3.equals(s1));

            const [sseq2, sseq3] = await i2.opMyStructSeq([s1]);
            test(sseq2[0].equals(s1));
            test(sseq3[0].equals(s1));

            const smap1 = new Map([["a", s1]]);
            const [smap2, smap3] = await i2.opMyStructMap(smap1);
            test(smap2.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            const c1 = new Test.Inner.Inner2.MyClass(s1);

            const [c2, c3] = await i2.opMyClass(c1);
            test(c2.s.equals(s1));
            test(c3.s.equals(s1));

            const [cseq2, cseq3] = await i2.opMyClassSeq([c1]);
            test(cseq2[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            const cmap1 = new Map([["a", c1]]);
            const [cmap2, cmap3] = await i2.opMyClassMap(cmap1);
            test(cmap2.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));
        }

        {
            const i3 = new Test.Inner.MyInterfacePrx(communicator, `i3:${this.getTestEndpoint()}`);
            const s1 = new Test.Inner.Inner2.MyStruct(0);

            const [s2, s3] = await i3.opMyStruct(s1);
            test(s2.equals(s1));
            test(s3.equals(s1));

            const [sseq2, sseq3] = await i3.opMyStructSeq([s1]);
            test(sseq2[0].equals(s1));
            test(sseq3[0].equals(s1));

            const smap1 = new Map([["a", s1]]);
            const [smap2, smap3] = await i3.opMyStructMap(smap1);
            test(smap2.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            const c1 = new Test.Inner.Inner2.MyClass(s1);

            const [c2, c3] = await i3.opMyClass(c1);
            test(c2.s.equals(s1));
            test(c3.s.equals(s1));

            const [cseq2, cseq3] = await i3.opMyClassSeq([c1]);
            test(cseq2[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            const cmap1 = new Map([["a", c1]]);
            const [cmap2, cmap3] = await i3.opMyClassMap(cmap1);
            test(cmap2.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));
        }

        {
            const i4 = new Inner.Test.Inner2.MyInterfacePrx(communicator, `i4:${this.getTestEndpoint()}`);
            const s1 = new Test.MyStruct(0);

            const [s2, s3] = await i4.opMyStruct(s1);
            test(s2.equals(s1));
            test(s3.equals(s1));

            const [sseq2, sseq3] = await i4.opMyStructSeq([s1]);
            test(sseq2[0].equals(s1));
            test(sseq3[0].equals(s1));

            const smap1 = new Map([["a", s1]]);
            const [smap2, smap3] = await i4.opMyStructMap(smap1);
            test(smap2.get("a").equals(s1));
            test(smap3.get("a").equals(s1));

            const c1 = new Test.MyClass(s1);

            const [c2, c3] = await i4.opMyClass(c1);
            test(c2.s.equals(s1));
            test(c3.s.equals(s1));

            const [cseq2, cseq3] = await i4.opMyClassSeq([c1]);
            test(cseq2[0].s.equals(s1));
            test(cseq3[0].s.equals(s1));

            const cmap1 = new Map([["a", c1]]);
            const [cmap2, cmap3] = await i4.opMyClassMap(cmap1);
            test(cmap2.get("a").s.equals(s1));
            test(cmap3.get("a").s.equals(s1));
        }

        {
            const i1 = new Test.MyInterfacePrx(communicator, `i1:${this.getTestEndpoint()}`);
            await i1.shutdown();
        }

        out.writeLine("ok");
    }

    async run(args: string[]) {
        let communicator: Ice.Communicator | null = null;
        try {
            [communicator] = this.initialize(args);
            await this.allTests();
        } finally {
            if (communicator) {
                await communicator.destroy();
            }
        }
    }
}
