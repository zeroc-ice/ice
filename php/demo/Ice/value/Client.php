<?php
// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

Ice_loadProfile();

class PrinterI extends Demo_Printer
{
    function printBackwards()
    {
        echo strrev($this->message),"\n";
    }
}

class DerivedPrinterI extends Demo_DerivedPrinter
{
    function printBackwards()
    {
        echo strrev($this->message),"\n";
    }

    function printUpperCase()
    {
        echo strtoupper($this->derivedMessage),"\n";
    }
}

class ObjectFactory extends Ice_LocalObjectImpl implements Ice_ObjectFactory
{
    function create($type)
    {
        if($type == "::Demo::Printer")
        {
            return new PrinterI;
        }

        if($type == "::Demo::DerivedPrinter")
        {
            return new DerivedPrinterI;
        }

        assert(false);
        return null;
    }

    function destroy()
    {
    }
}

try
{
    $base = $ICE->stringToProxy("initial:default -p 10000");
    $initial = $base->ice_checkedCast("::Demo::Initial");

    echo "\n";
    echo "Let's first transfer a simple object, for a class without\n";
    echo "operations, and print its contents. No factory is required\n";
    echo "for this.\n";
    echo "[press enter]\n";
    fgets(STDIN);

    $simple = $initial->getSimple();
    echo "==> ",$simple->message,"\n";

    echo "\n";
    echo "Yes, this worked. Now let's try to transfer an object for a class\n";
    echo "with operations as type ::Printer, without installing a factory first.\n";
    echo "This should give us a `no factory' exception.\n";
    echo "[press enter]\n";
    fgets(STDIN);

    try
    {
        $initial->getPrinter($printer, $printerProxy);
        die("Did not get the expected NoObjectFactoryException!");
    }
    catch(Ice_NoObjectFactoryException $ex)
    {
        print_r($ex);
    }

    echo "\n";
    echo "Yep, that's what we expected. Now let's try again, but with\n";
    echo "installing an appropriate factory first. If successful, we print\n";
    echo "the object's content.\n";
    echo "[press enter]\n";
    fgets(STDIN);

    $factory = new ObjectFactory;
    $ICE->addObjectFactory($factory, "::Demo::Printer");

    $initial->getPrinter($printer, $printerProxy);
    echo "==> ",$printer->message,"\n";

    echo "\n";;
    echo "Cool, it worked! Let's try calling the printBackwards() method\n";
    echo "on the object we just received locally.\n";
    echo "[press enter]\n";
    fgets(STDIN);

    echo "==> ";
    $printer->printBackwards();

    echo "\n";
    echo "Now we call the same method, but on the remote object. Watch the\n";
    echo "server's output.\n";
    echo "[press enter]\n";
    fgets(STDIN);

    $printerProxy->printBackwards();

    echo "\n";
    echo "Next, we transfer a derived object from the server as a base\n";
    echo "object. Since we haven't yet installed a factory for the derived\n";
    echo "class, the derived class (::Demo::DerivedPrinter) is sliced\n";
    echo "to its base class (::Demo::Printer).\n";
    echo "[press enter]\n";
    fgets(STDIN);

    $derivedAsBase = $initial->getDerivedPrinter();
    echo "==> The type ID of the received object is \"",get_class($derivedAsBase),"\"\n";
    assert($derivedAsBase instanceof Demo_Printer);

    echo "\n";
    echo "Now we install a factory for the derived class, and try again.\n";
    echo "Because we receive the derived object as a base object, we\n";
    echo "we need to do a dynamic_cast<> to get from the base to the derived object.\n";
    echo "[press enter]\n";
    fgets(STDIN);

    $ICE->addObjectFactory($factory, "::Demo::DerivedPrinter");

    $derivedAsBase = $initial->getDerivedPrinter();
    assert($derivedAsBase instanceof Demo_DerivedPrinter);
    $derived = $derivedAsBase;
    echo "==> dynamic_cast<> to derived object succeeded\n";
    echo "==> The type ID of the received object is \"",get_class($derived),"\"\n";

    echo "\n";
    echo "Let's print the message contained in the derived object, and\n";
    echo "call the operation printUppercase() on the derived object\n";
    echo "locally.\n";
    echo "[press enter]\n";
    fgets(STDIN);

    echo "==> ",$derived->derivedMessage,"\n";
    echo "==> ";
    $derived->printUppercase();

    echo "\n";
    echo "Finally, we try the same again, but instead of returning the\n";
    echo "derived object, we throw an exception containing the derived\n";
    echo "object.\n";
    echo "[press enter]\n";
    fgets(STDIN);

    try
    {
        $initial->throwDerivedPrinter();
        die("Did not get the expected DerivedPrinterException!");
    }
    catch(Demo_DerivedPrinterException $ex)
    {
        $derived = $ex->derived;
        assert($derived != null);
    }

    echo "==> ",$derived->derivedMessage,"\n";
    echo "==> ";
    $derived->printUppercase();

    echo "\n";
    echo "That's it for this demo. Have fun with Ice!\n";

    $initial->shutdown();
}
catch(Ice_LocalException $ex)
{
    print_r($ex);
}
?>
