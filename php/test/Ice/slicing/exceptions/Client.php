<?
error_reporting(E_ALL | E_STRICT);

if(!extension_loaded("ice"))
{
    echo "\nerror: Ice extension is not loaded.\n\n";
    exit(1);
}
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

    $obj = $ICE->stringToProxy("Test:default -p 12010");
    $test = $obj->ice_checkedCast("::Test::TestIntf");

    echo "testing throwing a base exception... ";
    flush();
    {
        try
        {
            $test->baseAsBase();
            test(false);
        }
        catch(Test_Base $b)
        {
            test($b->b == "Base.b");
            test(get_class($b) == "Test_Base");
        }
    }
    echo "ok\n";

    echo "testing slicing of unknown derived exception... ";
    flush();
    {
        try
        {
            $test->unknownDerivedAsBase();
            test(false);
        }
        catch(Test_Base $b)
        {
            test($b->b == "UnknownDerived.b");
            test(get_class($b) == "Test_Base");
        }
    }
    echo "ok\n";

    echo "testing non-slicing of known derived exception thrown as base exception... ";
    flush();
    {
        try
        {
            $test->knownDerivedAsBase();
            test(false);
        }
        catch(Test_KnownDerived $k)
        {
            test($k->b == "KnownDerived.b");
            test($k->kd == "KnownDerived.kd");
            test(get_class($k) == "Test_KnownDerived");
        }
    }
    echo "ok\n";

    echo "testing non-slicing of known derived exception thrown as derived exception... ";
    flush();
    {
        try
        {
            $test->knownDerivedAsKnownDerived();
            test(false);
        }
        catch(Test_KnownDerived $k)
        {
            test($k->b == "KnownDerived.b");
            test($k->kd == "KnownDerived.kd");
            test(get_class($k) == "Test_KnownDerived");
        }
    }
    echo "ok\n";

    echo "testing slicing of unknown intermediate exception thrown as base exception... ";
    flush();
    {
        try
        {
            $test->unknownIntermediateAsBase();
            test(false);
        }
        catch(Test_Base $b)
        {
            test($b->b == "UnknownIntermediate.b");
            test(get_class($b) == "Test_Base");
        }
    }
    echo "ok\n";

    echo "testing slicing of known intermediate exception thrown as base exception... ";
    flush();
    {
        try
        {
            $test->knownIntermediateAsBase();
            test(false);
        }
        catch(Test_KnownIntermediate $ki)
        {
            test($ki->b == "KnownIntermediate.b");
            test($ki->ki == "KnownIntermediate.ki");
            test(get_class($ki) == "Test_KnownIntermediate");
        }
    }
    echo "ok\n";

    echo "testing slicing of known most derived exception thrown as base exception... ";
    flush();
    {
        try
        {
            $test->knownMostDerivedAsBase();
            test(false);
        }
        catch(Test_KnownMostDerived $kmd)
        {
            test($kmd->b == "KnownMostDerived.b");
            test($kmd->ki == "KnownMostDerived.ki");
            test($kmd->kmd == "KnownMostDerived.kmd");
            test(get_class($kmd) == "Test_KnownMostDerived");
        }
    }
    echo "ok\n";

    echo "testing non-slicing of known intermediate exception thrown as intermediate exception... ";
    flush();
    {
        try
        {
            $test->knownIntermediateAsKnownIntermediate();
            test(false);
        }
        catch(Test_KnownIntermediate $ki)
        {
            test($ki->b == "KnownIntermediate.b");
            test($ki->ki == "KnownIntermediate.ki");
            test(get_class($ki) == "Test_KnownIntermediate");
        }
    }
    echo "ok\n";

    echo "testing non-slicing of known most derived thrown as intermediate exception... ";
    flush();
    {
        try
        {
            $test->knownMostDerivedAsKnownIntermediate();
            test(false);
        }
        catch(Test_KnownMostDerived $kmd)
        {
            test($kmd->b == "KnownMostDerived.b");
            test($kmd->ki == "KnownMostDerived.ki");
            test($kmd->kmd == "KnownMostDerived.kmd");
            test(get_class($kmd) == "Test_KnownMostDerived");
        }
    }
    echo "ok\n";

    echo "testing non-slicing of known most derived thrown as most derived exception... ";
    flush();
    {
        try
        {
            $test->knownMostDerivedAsKnownMostDerived();
            test(false);
        }
        catch(Test_KnownMostDerived $kmd)
        {
            test($kmd->b == "KnownMostDerived.b");
            test($kmd->ki == "KnownMostDerived.ki");
            test($kmd->kmd == "KnownMostDerived.kmd");
            test(get_class($kmd) == "Test_KnownMostDerived");
        }
    }
    echo "ok\n";

    echo "testing slicing of unknown most derived with known intermediate thrown as base exception... ";
    flush();
    {
        try
        {
            $test->unknownMostDerived1AsBase();
            test(false);
        }
        catch(Test_KnownIntermediate $ki)
        {
            test($ki->b == "UnknownMostDerived1.b");
            test($ki->ki == "UnknownMostDerived1.ki");
            test(get_class($ki) == "Test_KnownIntermediate");
        }
    }
    echo "ok\n";

    echo "testing slicing of unknown most derived with known intermediate thrown as intermediate exception... ";
    flush();
    {
        try
        {
            $test->unknownMostDerived1AsKnownIntermediate();
            test(false);
        }
        catch(Test_KnownIntermediate $ki)
        {
            test($ki->b == "UnknownMostDerived1.b");
            test($ki->ki == "UnknownMostDerived1.ki");
            test(get_class($ki) == "Test_KnownIntermediate");
        }
    }
    echo "ok\n";

    echo "testing slicing of unknown most derived with unknown intermediate thrown as base exception... ";
    flush();
    {
        try
        {
            $test->unknownMostDerived2AsBase();
            test(false);
        }
        catch(Test_Base $b)
        {
            test($b->b == "UnknownMostDerived2.b");
            test(get_class($b) == "Test_Base");
        }
    }
    echo "ok\n";

    return $test;
}

$test = allTests();
$test->shutdown();
exit();
?>
