# Ice for PHP

[Examples] | [Ice Manual] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for PHP is the PHP implementation of the Ice framework. As of this version, it
supports only clients: you need to implement the server portion of your application in
another programming language such as C++, C# or Java.

## Sample Code

```slice
// Slice definitions (Greeter.ice)

#pragma once

module VisitorCenter
{
    /// Represents a simple greeter.
    interface Greeter
    {
        /// Creates a personalized greeting.
        /// @param name The name of the person to greet.
        /// @return The greeting.
        string greet(string name);
    }
}
```

```php
<?php
// Client application (Client.php)

require_once 'Ice.php';
require_once 'Greeter.php';

$communicator = Ice\initialize();
$greeter = VisitorCenter\GreeterPrxHelper::createProxy($communicator, 'greeter:tcp -h localhost -p 4061');

$greeting = $greeter->greet(get_current_user());
echo "$greeting\n";
?>
```

[Examples]: https://github.com/zeroc-ice/ice-demos/tree/main/php
[Ice Manual]: https://doc.zeroc.com/ice/3.7
[Building from source]: ./BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
