// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var Promise = Ice.Promise;

    var test = function(b)
    {
        if(!b)
        {
            throw new Error("test failed");
        }
    };

    var run = function(out)
    {
        return Promise.try(() =>
            {
                out.write("testing configuration file escapes... ");
                var props =
                    {
                        "Foo\tBar": "3",
                        "Foo\\tBar": "4",
                        "Escape\\ Space": "2",
                        "Prop1": "1",
                        "Prop2": "2",
                        "Prop3": "3",
                        "My Prop1": "1",
                        "My Prop2": "2",
                        "My.Prop1": "a property",
                        "My.Prop2": "a     property",
                        "My.Prop3": "  a     property  ",
                        "My.Prop4": "  a     property  ",
                        "My.Prop5": "a \\ property",
                        "foo=bar": "1",
                        "foo#bar": "2",
                        "foo bar": "3",
                        "A": "1",
                        "B": "2 3 4",
                        "C": "5=#6",
                        "AServer": "\\\\server\\dir",
                        "BServer": "\\server\\dir"
                    };

                var properties = Ice.createProperties();
                if(typeof(require("fs").readFileSync) == "function")
                {
                    //
                    // We are runing with NodeJS we load the properties file from the file system.
                    //
                    properties.parse(require("fs").readFileSync(process.argv[3] + "/config/escapes.cfg", {encoding: "utf8"}));
                    for(var key in props)
                    {
                        test(props[key] == properties.getProperty(key));
                    }
                }
                else
                {
                    if(typeof window !== 'undefined')
                    {
                        //
                        //Skiped when running in a worker, we don't load JQuery in the workers
                        //

                        //
                        // We are runing in a web browser load the properties file from the web server.
                        //
                        var p = new Promise();
                        /*jshint jquery: true */
                        $.ajax(
                            {
                                url: "config/escapes.cfg",
                                //
                                // Use text data type to avoid problems interpreting the data.
                                //
                                dataType: "text"
                            }).done(data =>
                                {
                                    properties.parse(data);
                                    for(var key in props)
                                    {
                                        test(props[key] == properties.getProperty(key));
                                    }
                                    p.resolve();
                                }).fail(p.reject);
                        return p;
                    }
                }
            }
        ).then(() => out.writeLine("ok"));
    };
    exports._test = run;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
