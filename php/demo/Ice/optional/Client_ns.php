<?php
// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

require_once 'Ice_ns.php';
require_once 'Contact.php';

$ICE = Ice\initialize();

try
{
    $base = $ICE->stringToProxy("contactdb:default -h localhost -p 10000");
    $contactdb = Demo\ContactDBPrxHelper::checkedCast($base);

    //
    // Add a contact for "john". All parameters are provided.
    //
    $johnNumber = "123-456-7890";
    $contactdb->addContact("john", Demo\NumberType::HOME, $johnNumber, 0);

    echo "Checking john... ";

    //
    // Find the phone number for "john"
    //
    $number = $contactdb->queryNumber("john");
    if($number != $johnNumber)
    {
        echo "number is incorrect ";
    }

    // Optional can also be used in an out parameter.
    $dialgroup = Ice_Unset;
    $contactdb->queryDialgroup("john", $dialgroup);
    if($dialgroup != 0)
    {
        echo "dialgroup is incorrect ";
    }

    $info = $contactdb->query("john");
    //
    // All of the info parameters should be set.
    //
    if($info->type != Demo\NumberType::HOME || $info->number != $johnNumber || $info->dialGroup != 0)
    {
        echo "info is incorrect ";
    }
    echo "ok\n";

    //
    // Add a contact for "steve". The behavior of the server is to
    // default construct the Contact, and then assign all set parameters.
    // Since the default value of NumberType in the slice definition
    // is HOME and in this case the NumberType is unset it will take
    // the default value.
    //
    $steveNumber = "234-567-8901";
    $contactdb->addContact("steve", Ice_Unset, $steveNumber, 1);

    echo "Checking steve... ";
    $number = $contactdb->queryNumber("steve");
    if($number != $steveNumber)
    {
        echo "number is incorrect ";
    }

    $info = $contactdb->query("steve");
    //
    // Check the value for the NumberType.
    //
    if($info->type != Demo\NumberType::HOME)
    {
        echo "info is incorrect ";
    }

    if($info->number != $steveNumber || $info->dialGroup != 1)
    {
        echo "info is incorrect ";
    }

    $contactdb->queryDialgroup("steve", $dialgroup);
    if($dialgroup != 1)
    {
        echo "dialgroup is incorrect ";
    }

    echo "ok\n";

    //
    // Add a contact from "frank". Here the dialGroup field isn't set.
    //
    $frankNumber = "345-678-9012";
    $contactdb->addContact("frank", Demo\NumberType::CELL, $frankNumber, Ice_Unset);

    echo "Checking frank... ";

    $number = $contactdb->queryNumber("frank");
    if($number != $frankNumber)
    {
        echo "number is incorrect ";
    }

    $info = $contactdb->query("frank");
    //
    // The dial group field should be unset.
    //
    if($info->dialGroup != Ice_Unset)
    {
        echo "info is incorrect ";
    }
    if($info->type != Demo\NumberType::CELL || $info->number != $frankNumber)
    {
        echo "info is incorrect ";
    }

    $contactdb->queryDialgroup("frank", $dialgroup);
    if($dialgroup != Ice_Unset)
    {
        echo "dialgroup is incorrect ";
    }
    echo "ok\n";

    //
    // Add a contact from "anne". The number field isn't set.
    //
    $contactdb->addContact("anne", Demo\NumberType::OFFICE, Ice_Unset, 2);

    echo "Checking anne... ";
    $number = $contactdb->queryNumber("anne");
    if($number != Ice_Unset)
    {
        echo "number is incorrect ";
    }

    $info = $contactdb->query("anne");
    //
    // The number field should be unset.
    //
    if($info->number != Ice_Unset)
    {
        echo "info is incorrect ";
    }
    if($info->type != Demo\NumberType::OFFICE || $info->dialGroup != 2)
    {
        echo "info is incorrect ";
    }

    $contactdb->queryDialgroup("anne", $dialgroup);
    if($dialgroup != 2)
    {
        echo "dialgroup is incorrect ";
    }

    //
    // The optional fields can be used to determine what fields to
    // update on the contact.  Here we update only the number for anne,
    // the remainder of the fields are unchanged.
    //
    $anneNumber = "456-789-0123";
    $contactdb->updateContact("anne", Ice_Unset, $anneNumber, Ice_Unset);
    $number = $contactdb->queryNumber("anne");
    if($number != $anneNumber)
    {
        echo "number is incorrect ";
    }
    $info = $contactdb->query("anne");
    if($info->number != $anneNumber || $info->type != Demo\NumberType::OFFICE || $info->dialGroup != 2)
    {
        echo "info is incorrect ";
    }
    echo "ok\n";

    $contactdb->shutdown();
}
catch(Ice_LocalException $ex)
{
    print_r($ex);
}
?>
