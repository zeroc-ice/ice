# Ice for PHP

[Getting started] | [Examples] | [Documentation] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for PHP is the PHP implementation of the Ice framework.

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

```php
// Client application (Client.php)
require_once 'Ice.php';
require_once 'Hello.php';

$communicator = Ice\initialize();
$hello = Demo\HelloPrxHelper::uncheckedCast(
    $communicator->stringToProxy("hello:default -h localhost -p 10000"));
$hello->sayHello();
```

[Getting started]: https://doc.zeroc.com/ice/3.7/hello-world-application/writing-an-ice-application-with-php
[Examples]: https://github.com/zeroc-ice/ice-demos/tree/3.7/php
[Documentation]: https://doc.zeroc.com/ice/3.7
[Building from source]: https://github.com/zeroc-ice/ice/blob/3.7/php/BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
