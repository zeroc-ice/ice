<?
Ice_loadProfileWithArgs($argv);

function test($b)
{
    if(!$b)
    {
        $bt = debug_backtrace();
        die("\ntest failed in ".$bt[0]["file"]." line ".$bt[0]["line"]."\n");
    }
}

function allTests()
{
    global $ICE;

    $obj = $ICE->stringToProxy("Test:default -p 12345");
    $test = $obj->ice_checkedCast("::Test");

    echo "testing throwing a base exception... ";
    flush();
    {
        $gotException = false;
        try
        {
            $test->baseAsBase();
        }
        catch(Base $b)
        {
            test($b->b == "Base.b");
            test(get_class($b) == "Base");
            $gotException = true;
        }
        test($gotException);
    }
    echo "ok\n";

    echo "testing slicing of unknown derived exception... ";
    flush();
    {
        $gotException = false;
        try
        {
            $test->unknownDerivedAsBase();
        }
        catch(Base $b)
        {
            test($b->b == "UnknownDerived.b");
            test(get_class($b) == "Base");
            $gotException = true;
        }
        test($gotException);
    }
    echo "ok\n";

    echo "testing non-slicing of known derived exception thrown as base exception... ";
    flush();
    {
        $gotException = false;
        try
        {
            $test->knownDerivedAsBase();
        }
        catch(KnownDerived $k)
        {
            test($k->b == "KnownDerived.b");
            test($k->kd == "KnownDerived.kd");
            test(get_class($k) == "KnownDerived");
            $gotException = true;
        }
        test($gotException);
    }
    echo "ok\n";

    echo "testing non-slicing of known derived exception thrown as derived exception... ";
    flush();
    {
        $gotException = false;
        try
        {
            $test->knownDerivedAsKnownDerived();
        }
        catch(KnownDerived $k)
        {
            test($k->b == "KnownDerived.b");
            test($k->kd == "KnownDerived.kd");
            test(get_class($k) == "KnownDerived");
            $gotException = true;
        }
        test($gotException);
    }
    echo "ok\n";

    echo "testing slicing of unknown intermediate exception thrown as base exception... ";
    flush();
    {
        $gotException = false;
        try
        {
            $test->unknownIntermediateAsBase();
        }
        catch(Base $b)
        {
            test($b->b == "UnknownIntermediate.b");
            test(get_class($b) == "Base");
            $gotException = true;
        }
        test($gotException);
    }
    echo "ok\n";

    echo "testing slicing of known intermediate exception thrown as base exception... ";
    flush();
    {
        $gotException = false;
        try
        {
            $test->knownIntermediateAsBase();
        }
        catch(KnownIntermediate $ki)
        {
            test($ki->b == "KnownIntermediate.b");
            test($ki->ki == "KnownIntermediate.ki");
            test(get_class($ki) == "KnownIntermediate");
            $gotException = true;
        }
        test($gotException);
    }
    echo "ok\n";

    echo "testing slicing of known most derived exception thrown as base exception... ";
    flush();
    {
        $gotException = false;
        try
        {
            $test->knownMostDerivedAsBase();
        }
        catch(KnownMostDerived $kmd)
        {
            test($kmd->b == "KnownMostDerived.b");
            test($kmd->ki == "KnownMostDerived.ki");
            test($kmd->kmd == "KnownMostDerived.kmd");
            test(get_class($kmd) == "KnownMostDerived");
            $gotException = true;
        }
        test($gotException);
    }
    echo "ok\n";

    echo "testing non-slicing of known intermediate exception thrown as intermediate exception... ";
    flush();
    {
        $gotException = false;
        try
        {
            $test->knownIntermediateAsKnownIntermediate();
        }
        catch(KnownIntermediate $ki)
        {
            test($ki->b == "KnownIntermediate.b");
            test($ki->ki == "KnownIntermediate.ki");
            test(get_class($ki) == "KnownIntermediate");
            $gotException = true;
        }
        test($gotException);
    }
    echo "ok\n";

    echo "testing non-slicing of known most derived thrown as intermediate exception... ";
    flush();
    {
        $gotException = false;
        try
        {
            $test->knownMostDerivedAsKnownIntermediate();
        }
        catch(KnownMostDerived $kmd)
        {
            test($kmd->b == "KnownMostDerived.b");
            test($kmd->ki == "KnownMostDerived.ki");
            test($kmd->kmd == "KnownMostDerived.kmd");
            test(get_class($kmd) == "KnownMostDerived");
            $gotException = true;
        }
        test($gotException);
    }
    echo "ok\n";

    echo "testing non-slicing of known most derived thrown as most derived exception... ";
    flush();
    {
        $gotException = false;
        try
        {
            $test->knownMostDerivedAsKnownMostDerived();
        }
        catch(KnownMostDerived $kmd)
        {
            test($kmd->b == "KnownMostDerived.b");
            test($kmd->ki == "KnownMostDerived.ki");
            test($kmd->kmd == "KnownMostDerived.kmd");
            test(get_class($kmd) == "KnownMostDerived");
            $gotException = true;
        }
        test($gotException);
    }
    echo "ok\n";

    echo "testing slicing of unknown most derived with known intermediate thrown as base exception... ";
    flush();
    {
        $gotException = false;
        try
        {
            $test->unknownMostDerived1AsBase();
        }
        catch(KnownIntermediate $ki)
        {
            test($ki->b == "UnknownMostDerived1.b");
            test($ki->ki == "UnknownMostDerived1.ki");
            test(get_class($ki) == "KnownIntermediate");
            $gotException = true;
        }
        test($gotException);
    }
    echo "ok\n";

    echo "testing slicing of unknown most derived with known intermediate thrown as intermediate exception... ";
    flush();
    {
        $gotException = false;
        try
        {
            $test->unknownMostDerived1AsKnownIntermediate();
        }
        catch(KnownIntermediate $ki)
        {
            test($ki->b == "UnknownMostDerived1.b");
            test($ki->ki == "UnknownMostDerived1.ki");
            test(get_class($ki) == "KnownIntermediate");
            $gotException = true;
        }
        test($gotException);
    }
    echo "ok\n";

    echo "testing slicing of unknown most derived with unknown intermediate thrown as base exception... ";
    flush();
    {
        $gotException = false;
        try
        {
            $test->unknownMostDerived2AsBase();
        }
        catch(Base $b)
        {
            test($b->b == "UnknownMostDerived2.b");
            test(get_class($b) == "Base");
            $gotException = true;
        }
        test($gotException);
    }
    echo "ok\n";

    return $test;
}

$test = allTests();
$test->shutdown();
exit();
?>
