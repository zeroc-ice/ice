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

        // An expiration time whose value in milliseconds exceeds INT32_MAX is accepted.
        Ice\register($communicator, "Hello7", 40000);
        test(Ice\find('Hello7') != null);
        test(Ice\unregister('Hello7'));

        // An out-of-range expiration time is rejected.
        try {
            Ice\register($communicator, "Hello8", 1e300);
            test(false);
        } catch (\InvalidArgumentException $ex) {
        }
        test(Ice\find('Hello8') == null);

        // A registered communicator must not be reaped before its expiration time. The reap task runs every
        // expires/2; intermediate ticks must not destroy a communicator that has not yet expired.
        $communicator = Ice\initialize();
        Ice\register($communicator, "Reap", 0.1); // Expires after ~6s; the reap task ticks every ~3s.
        sleep(4); // Past the first reap tick (~3s) but well before expiration (~6s).
        $reapCommunicator = Ice\find("Reap");
        test($reapCommunicator != null);
        // The communicator has not expired, so it must still be usable.
        $reapCommunicator->stringToProxy("test");
        $reapCommunicator->destroy();
        test(Ice\find("Reap") == null);

        // A communicator registered with an expiration time and never unregistered must still be destroyed at module
        // shutdown. The reap task holds the only remaining reference at that point, so if it isn't cancelled the
        // communicator survives and PHP reports "communicator not destroyed during global destruction."
        $communicator = Ice\initialize();
        Ice\register($communicator, "Hello9", 40000);
        test(Ice\find('Hello9') != null);

        // A proxy keeps its communicator alive at the C++ level, but not the PHP object that wraps it. Once that PHP
        // object is freed, ice_getCommunicator must return a working communicator by re-wrapping the same underlying
        // communicator.
        $proxy = (function () {
            $c = Ice\initialize();
            return $c->stringToProxy("test:tcp -h 127.0.0.1 -p 10000");
        })();
        $comm = $proxy->ice_getCommunicator();
        test($comm != null);
        test($comm->stringToProxy("test") != null);
        // A second call returns the same re-wrapped communicator.
        test($proxy->ice_getCommunicator() === $comm);
        $comm->destroy();
    }
}
