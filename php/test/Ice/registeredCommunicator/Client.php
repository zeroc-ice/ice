<?php
// Copyright (c) ZeroC, Inc.

class Client extends TestHelper
{
    function run($args)
    {
        $communicator = Ice\initialize();
        Ice\register($communicator, "Hello1", 1);
        // Register the same communicator with a second Id and a different expiration time. The last expiration time
        // takes precedence.
        Ice\register($communicator, "Hello2", 0.016);

        // Register a separate communicator with Hello3 Id, without expiration time.
        $communicator = Ice\initialize();
        Ice\register($communicator, "Hello3");

        $helloCommunicator = Ice\find('Hello1');
        test($helloCommunicator != null);
        $hello2Communicator = Ice\find('Hello2');
        test($hello2Communicator == $helloCommunicator);

        // Sleep for 2 seconds the Hello1 and Hello2 registrations will expires after 0.016 minutes ~= 1s.
        // The Hello3 registration doesn't expires.
        sleep(2);

        test(Ice\find('Hello1') == null);
        test(Ice\find('Hello2') == null);
        test(Ice\find('Hello3') != null);
    }
}
