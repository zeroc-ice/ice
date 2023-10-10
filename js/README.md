# Ice For JavaScript

[Getting started] | [Examples] | [NPM packages] | [Documentation] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for JavaScript is the JavaScript implementation of the Ice framework.

## Sample Code

```slice
// Slice definitions (Hello.ice)

module Demo
{
    interface Hello
    {
        void sayHello();
    }
}
```

```javascript
// Client application (client.js)
let communicator;
try
{
    communicator = Ice.initialize(process.argv);
    const hello = await Demo.HelloPrx.checkedCast(
        communicator.stringToProxy("hello:tcp -h localhost -p 10000"));
    await hello.sayHello();
}
catch(ex)
{
    console.log(ex.toString());
}
finally
{
   if(communicator)
   {
      await communicator.destroy();
   }
}
```

[Getting started]: https://doc.zeroc.com/ice/3.7/hello-world-application/writing-an-ice-application-with-javascript
[Examples]: https://github.com/zeroc-ice/ice-demos/tree/3.7/js
[NPM Packages]: https://www.npmjs.com/~zeroc
[Documentation]: https://doc.zeroc.com/ice/3.7
[Building from source]: https://github.com/zeroc-ice/ice/blob/3.7/js/BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice