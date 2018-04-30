// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

/* global _test, _runServer, _server, Test, WorkerGlobalScope, Ice */
/* eslint no-unused-vars: ["error", { "varsIgnorePattern": "isSafari|isChrome|isWorker|isWindows|runTest" }] */

function isSafari()
{
    return (/^((?!chrome).)*safari/i).test(navigator.userAgent);
}

function isChrome()
{
    //
    // We need to check for Edge browser as it might include Chrome in its user agent.
    //
    return navigator.userAgent.indexOf("Edge/") === -1 &&
           navigator.userAgent.indexOf("Chrome/") !== -1;
}

function isWorker()
{
    return typeof WorkerGlobalScope !== 'undefined' && this instanceof WorkerGlobalScope;
}

function isWindows()
{
    return navigator.userAgent.indexOf("Windows") != -1;
}

async function runTest(testsuite, language, host, protocol, testcases, out)
{
    //
    // This logger is setup to work with Web Workers and normal scripts using
    // the received out object. With some browser like Safari using console.log
    // method doesn't work when running inside a web worker.
    //
    class Logger
    {
        print(message)
        {
            out.writeLine(message, false);
        }

        trace(category, message)
        {
            const s = [];
            s.push("-- ");
            s.push(this.timestamp());
            s.push(' ');
            s.push(this._prefix);
            s.push(category);
            s.push(": ");
            s.push(message);
            out.writeLine(s.join(""), true);
        }

        warning(message)
        {
            const s = [];
            s.push("-! ");
            s.push(this.timestamp());
            s.push(' ');
            s.push(this._prefix);
            s.push("warning: ");
            s.push(message);
            out.writeLine(s.join(""), true);
        }

        error(message)
        {
            const s = [];
            s.push("!! ");
            s.push(this.timestamp());
            s.push(' ');
            s.push(this._prefix);
            s.push("error: ");
            s.push(message);
            out.writeLine(s.join(""), true);
        }

        getPrefix()
        {
            return "";
        }

        cloneWithPrefix()
        {
            return Logger;
        }

        timestamp()
        {
            const d = new Date();
            return d.toLocaleString("en-US", this._dateformat) + "." + d.getMilliseconds();
        }
    }

    let communicator;
    try
    {
        const initData = new Ice.InitializationData();
        const port = protocol == "ws" ? 15002 : 15003;
        initData.logger = new Logger();
        initData.properties = Ice.createProperties();
        initData.properties.setProperty("Ice.Default.Host", host);
        initData.properties.setProperty("Ice.Default.Protocol", protocol);
        // initData.properties.setProperty("Ice.Trace.Protocol", "1");
        // initData.properties.setProperty("Ice.Trace.Network", "3");

        if(typeof _runServer === "undefined")
        {
            await _test(out, initData);
        }
        else
        {
            communicator = Ice.initialize();
            const controller = Test.Common.ControllerPrx.uncheckedCast(
                communicator.stringToProxy(`controller:${protocol} -h ${host} -p ${port}`));

            testcases = testcases || [{name: "client/server"}];

            const run = async (testcase, client) =>
                  {
                      let serverTestCase;
                      try
                      {
                          if(testcase.langs && testcase.langs.indexOf(language) == -1)
                          {
                              return;
                          }

                          out.writeLine(`[ running ${testcase.name} test]`);
                          out.write("starting server side... ");
                          serverTestCase = (language === "js") ?
                              await controller.runTestCase("cpp", "Ice/echo", "server", "") :
                              await controller.runTestCase("js", testsuite, testcase.name, language);

                          serverTestCase = Test.Common.TestCasePrx.uncheckedCast(
                              controller.ice_getCachedConnection().createProxy(serverTestCase.ice_getIdentity()));

                          const config = new Test.Common.Config();
                          config.protocol = protocol;
                          await serverTestCase.startServerSide(config);
                          out.writeLine("ok");

                          let server;
                          if(language === "js")
                          {
                              const id = initData.clone();
                              if(testcase.args !== undefined)
                              {
                                  id.properties = Ice.createProperties(testcase.args, initData.properties);
                              }
                              const ready = new Ice.Promise();
                              server = _server(out, id, ready, testcase.args);
                              await ready;
                          }

                          {
                              const id = initData.clone();
                              if(testcase.args !== undefined)
                              {
                                  initData.properties = Ice.createProperties(testcase.args, id.properties);
                              }
                              await client(out, id, testcase.args);
                          }

                          if(server)
                          {
                              await server; // Wait for server to terminate
                          }

                          await serverTestCase.stopServerSide(true);
                      }
                      catch(ex)
                      {
                          out.writeLine("failed! (" + ex + ")");
                          throw ex;
                      }
                      finally
                      {
                          if(serverTestCase)
                          {
                              await serverTestCase.destroy();
                          }
                      }
                  };

            for(const testcase of testcases)
            {
                await run(testcase, _test);
            }
        }
    }
    catch(ex)
    {
        out.writeLine("");
        if(ex instanceof Test.Common.TestCaseFailedException)
        {
            out.writeLine("Server test case failed to start:\n");
            out.writeLine(ex.output);
        }
        else
        {
            out.writeLine(ex.toString());
            if(ex.stack)
            {
                out.writeLine(ex.stack);
            }
        }
        return false;
    }
    finally
    {
        if(communicator)
        {
            await communicator.destroy();
        }
    }
    return true;
}
