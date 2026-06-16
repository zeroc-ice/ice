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

        $communicator = Ice\initialize();
        Ice\register($communicator, "Hello4");
        Ice\register($communicator, "Hello5");
        test(Ice\find('Hello4') != null);
        test(Ice\find('Hello5') != null);
        // Destroying a communicator removes its registrations.
        $communicator->destroy();
        test(Ice\find('Hello4') == null);
        test(Ice\find('Hello5') == null);
        // Calling destroy again should have no effect.
        $communicator->destroy();

        // Cannot register a different communicator with the same ID.
        $communicator = Ice\initialize();
        Ice\register($communicator, "Hello6");
        test(Ice\find('Hello6') != null);
        $communicator = Ice\initialize();
        test(Ice\register($communicator, "Hello6") == false);

        // A registered communicator must not be reaped before its expiration time. The reap task runs every
        // expires/2; intermediate ticks must not destroy a communicator that has not yet expired (see #5533).
        $communicator = Ice\initialize();
        Ice\register($communicator, "Reap", 0.1); // Expires after ~6s; the reap task ticks every ~3s.
        sleep(4); // Past the first reap tick (~3s) but well before expiration (~6s).
        $reapCommunicator = Ice\find("Reap");
        test($reapCommunicator != null);
        // The communicator has not expired, so it must still be usable. Before #5533 it was destroyed on the
        // first reap tick and this call raised CommunicatorDestroyedException.
        $reapCommunicator->stringToProxy("test");
        $reapCommunicator->destroy();
        test(Ice\find("Reap") == null);
    }
}
