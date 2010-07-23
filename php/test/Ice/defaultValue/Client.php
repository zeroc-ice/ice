<?
// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

error_reporting(E_ALL | E_STRICT);

if(!extension_loaded("ice"))
{
    echo "\nerror: Ice extension is not loaded.\n\n";
    exit(1);
}

$NS = function_exists("Ice\\initialize");
require ($NS ? 'Ice_ns.php' : 'Ice.php');
require 'Test.php';

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
    global $NS;

    $red = $NS ? constant("Test\\Color::red") : constant("Test_Color::red");
    $green = $NS ? constant("Test\\Color::green") : constant("Test_Color::green");
    $blue = $NS ? constant("Test\\Color::blue") : constant("Test_Color::blue");

    echo "testing default values... ";
    flush();

    {
        $v = $NS ? eval("return new Test\\Struct1;") : eval("return new Test_Struct1;");
        test(!$v->boolFalse);
        test($v->boolTrue);
        test($v->b == 254);
        test($v->s == 16000);
        test($v->i == 3);
        test($v->l == 4);
        test($v->f == 5.1);
        test($v->d == 6.2);
        test($v->str == "foo bar");
        test($v->c == $red);
        test(strlen($v->noDefault) == 0);
    }

    {
        $v = $NS ? eval("return new Test\\Struct2;") : eval("return new Test_Struct2;");
        test($v->boolTrue == constant($NS ? "Test\\ConstBool" : "Test_ConstBool"));
        test($v->b == constant($NS ? "Test\\ConstByte" : "Test_ConstByte"));
        test($v->s == constant($NS ? "Test\\ConstShort" : "Test_ConstShort"));
        test($v->i == constant($NS ? "Test\\ConstInt" : "Test_ConstInt"));
        test($v->l == constant($NS ? "Test\\ConstLong" : "Test_ConstLong"));
        test($v->f == constant($NS ? "Test\\ConstFloat" : "Test_ConstFloat"));
        test($v->d == constant($NS ? "Test\\ConstDouble" : "Test_ConstDouble"));
        test($v->str == constant($NS ? "Test\\ConstString" : "Test_ConstString"));
        test($v->c == constant($NS ? "Test\\ConstColor" : "Test_ConstColor"));
    }

    {
        $v = $NS ? eval("return new Test\\Base;") : eval("return new Test_Base;");
        test(!$v->boolFalse);
        test($v->boolTrue);
        test($v->b == 1);
        test($v->s == 2);
        test($v->i == 3);
        test($v->l == 4);
        test($v->f == 5.1);
        test($v->d == 6.2);
        test($v->str == "foo bar");
        test(strlen($v->noDefault) == 0);
    }

    {
        $v = $NS ? eval("return new Test\\Derived;") : eval("return new Test_Derived;");
        test(!$v->boolFalse);
        test($v->boolTrue);
        test($v->b == 1);
        test($v->s == 2);
        test($v->i == 3);
        test($v->l == 4);
        test($v->f == 5.1);
        test($v->d == 6.2);
        test($v->str == "foo bar");
        test(strlen($v->noDefault) == 0);
        test($v->c == $green);
    }

    {
        $v = $NS ? eval("return new Test\\BaseEx;") : eval("return new Test_BaseEx;");
        test(!$v->boolFalse);
        test($v->boolTrue);
        test($v->b == 1);
        test($v->s == 2);
        test($v->i == 3);
        test($v->l == 4);
        test($v->f == 5.1);
        test($v->d == 6.2);
        test($v->str == "foo bar");
        test(strlen($v->noDefault) == 0);
    }

    {
        $v = $NS ? eval("return new Test\\DerivedEx;") : eval("return new Test_DerivedEx;");
        test(!$v->boolFalse);
        test($v->boolTrue);
        test($v->b == 1);
        test($v->s == 2);
        test($v->i == 3);
        test($v->l == 4);
        test($v->f == 5.1);
        test($v->d == 6.2);
        test($v->str == "foo bar");
        test(strlen($v->noDefault) == 0);
        test($v->c == $green);
    }
    echo "ok\n";
}

allTests();

exit();
?>
