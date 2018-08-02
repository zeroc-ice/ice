// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

/* eslint-env jquery */

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const TestHelper = require("TestHelper").TestHelper;
    const test = TestHelper.test;

    class Client extends TestHelper
    {
        async allTests(args)
        {
            const out = this.getWriter();
            out.write("testing configuration file escapes... ");
            const props = new Map();
            props.set("Foo\tBar", "3");
            props.set("Foo\\tBar", "4");
            props.set("Escape\\ Space", "2");
            props.set("Prop1", "1");
            props.set("Prop2", "2");
            props.set("Prop3", "3");
            props.set("My Prop1", "1");
            props.set("My Prop2", "2");
            props.set("My.Prop1", "a property");
            props.set("My.Prop2", "a     property");
            props.set("My.Prop3", "  a     property  ");
            props.set("My.Prop4", "  a     property  ");
            props.set("My.Prop5", "a \\ property");
            props.set("foo=bar", "1");
            props.set("foo#bar", "2");
            props.set("foo bar", "3");
            props.set("A", "1");
            props.set("B", "2 3 4");
            props.set("C", "5=#6");
            props.set("AServer", "\\\\server\\dir");
            props.set("BServer", "\\server\\dir");

            const properties = Ice.createProperties();
            /* eslint-disable no-sync */
            if(typeof require("fs").readFileSync == "function")
            {
                const path = require("path");
                //
                // We are runing with NodeJS we load the properties file from the file system.
                //
                properties.parse(require("fs").readFileSync(path.join(args[3], "config", "escapes.cfg"),
                                                            {encoding: "utf8"}));
                for(const [key, value] of props)
                {
                    test(properties.getProperty(key) == value);
                }
            }
            /* eslint-enable no-sync */
            else if(typeof window !== 'undefined')
            {
                //
                // Skiped when running in a worker, we don't load JQuery in the workers
                //

                //
                // We are runing in a web browser load the properties file from the web server.
                //
                await new Promise(
                    (resolve, reject) =>
                        {
                            //
                            // Use text data type to avoid problems interpreting the data.
                            //
                            $.ajax({url: "config/escapes.cfg", dataType: "text"}).done(
                                data =>
                                    {
                                        properties.parse(data);
                                        for(const [key, value] of props)
                                        {
                                            test(properties.getProperty(key) == value);
                                        }
                                        resolve();
                                    }).fail(reject);
                        });
            }
            out.writeLine("ok");
        }

        async run(args)
        {
            let communicator;
            try
            {
                communicator = this.initialize(args);
                await this.allTests(args);
            }
            finally
            {
                if(communicator)
                {
                    await communicator.destroy();
                }
            }
        }
    }
    exports.Client = Client;

}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require : this.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports : this));
