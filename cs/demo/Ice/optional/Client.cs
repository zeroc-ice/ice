// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceOptionalClient")]
[assembly: AssemblyDescription("Ice optional demo client")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public class App : Ice.Application
    {
        public override int run(string[] args)
        {
            if(args.Length > 0)
            {
                Console.Error.WriteLine(appName() + ": too many arguments");
                return 1;
            }

            ContactDBPrx contactdb = ContactDBPrxHelper.checkedCast(communicator().propertyToProxy("ContactDB.Proxy"));
            if(contactdb == null)
            {
                Console.Error.WriteLine(appName() + ": invalid proxy");
                return 1;
            }

            //
            // Add a contact for "john". All parameters are provided.
            //
            String johnNumber = "123-456-7890";
            contactdb.addContact("john", NumberType.HOME, johnNumber, 0);

            Console.Write("Checking john... ");

            //
            // Find the phone number for "john".
            //
            Ice.Optional<String> number = contactdb.queryNumber("john");

            //
            // HasValue tests if an optional value is set.
            //
            if(!number.HasValue)
            {
                Console.Write("number is incorrect ");
            }

            //
            // Call Value to retrieve the optional value.
            //
            if(!number.Value.Equals(johnNumber))
            {
                Console.Write("number is incorrect ");
            }

            // Optional can also be used in an out parameter.
            Ice.Optional<int> dialgroup;
            contactdb.queryDialgroup("john", out dialgroup);
            if(!dialgroup.HasValue || dialgroup.Value != 0)
            {
                Console.Write("dialgroup is incorrect ");
            }

            Contact info = contactdb.query("john");

            //
            // All of the info parameters should be set. Each of the optional members
            // of the class map to Ice.Optional<T> member.
            //
            if(!info.type.HasValue || !info.number.HasValue || !info.dialGroup.HasValue)
            {
                Console.Write("info is incorrect ");
            }
            if(info.type.Value != NumberType.HOME || !info.number.Value.Equals(johnNumber) || info.dialGroup.Value != 0)
            {
                Console.Write("info is incorrect ");
            }
            Console.WriteLine("ok");

            //
            // Add a contact for "steve". The behavior of the server is to
            // default construct the Contact, and then assign  all set parameters.
            // Since the default value of NumberType in the slice definition
            // is NumberType.HOME and in this case the NumberType is unset it will take
            // the default value.
            //
            // The C# mapping permits Ice.Util.None to be passed to unset optional values.
            //
            String steveNumber = "234-567-8901";
            contactdb.addContact("steve", Ice.Util.None, steveNumber, 1);

            Console.Write("Checking steve... ");
            number = contactdb.queryNumber("steve");
            if(!number.Value.Equals(steveNumber))
            {
                Console.Write("number is incorrect ");
            }

            info = contactdb.query("steve");
            //
            // Check the value for the NumberType.
            //
            if(!info.type.HasValue || info.type.Value != NumberType.HOME)
            {
                Console.Write("info is incorrect ");
            }

            if(!info.number.Value.Equals(steveNumber) || info.dialGroup.Value != 1)
            {
                Console.Write("info is incorrect ");
            }

            contactdb.queryDialgroup("steve", out dialgroup);
            if(!dialgroup.HasValue || dialgroup.Value != 1)
            {
                Console.Write("dialgroup is incorrect ");
            }

            Console.WriteLine("ok");

            //
            // Add a contact from "frank". Here the dialGroup field isn't set.
            //
            String frankNumber = "345-678-9012";
            contactdb.addContact("frank", NumberType.CELL, frankNumber, Ice.Util.None);

            Console.Write("Checking frank... ");

            number = contactdb.queryNumber("frank");
            if(!number.Value.Equals(frankNumber))
            {
                Console.Write("number is incorrect ");
            }

            info = contactdb.query("frank");
            //
            // The dial group field should be unset.
            //
            if(info.dialGroup.HasValue)
            {
                Console.Write("info is incorrect ");
            }
            if(info.type.Value != NumberType.CELL || !info.number.Value.Equals(frankNumber))
            {
                Console.Write("info is incorrect ");
            }

            contactdb.queryDialgroup("frank", out dialgroup);
            if(dialgroup.HasValue)
            {
                Console.Write("dialgroup is incorrect ");
            }
            Console.WriteLine("ok");

            //
            // Add a contact from "anne". The number field isn't set.
            //
            contactdb.addContact("anne", NumberType.OFFICE, Ice.Util.None, 2);

            Console.Write("Checking anne... ");
            number = contactdb.queryNumber("anne");
            if(number.HasValue)
            {
                Console.Write("number is incorrect ");
            }

            info = contactdb.query("anne");
            //
            // The number field should be unset.
            //
            if(info.number.HasValue)
            {
                Console.Write("info is incorrect ");
            }
            if(info.type.Value != NumberType.OFFICE || info.dialGroup.Value != 2)
            {
                Console.Write("info is incorrect ");
            }

            contactdb.queryDialgroup("anne", out dialgroup);
            if(!dialgroup.HasValue || dialgroup.Value != 2)
            {
                Console.Write("dialgroup is incorrect ");
            }

            //
            // The optional fields can be used to determine what fields to
            // update on the contact.  Here we update only the number for anne,
            // the remainder of the fields are unchanged.
            //
            String anneNumber = "456-789-0123";
            contactdb.updateContact("anne", Ice.Util.None, new Ice.Optional<String>(anneNumber), Ice.Util.None);
            number = contactdb.queryNumber("anne");
            if(!number.Value.Equals(anneNumber))
            {
                Console.Write("number is incorrect ");
            }
            info = contactdb.query("anne");
            if(!info.number.Value.Equals(anneNumber) || info.type.Value != NumberType.OFFICE || info.dialGroup.Value != 2)
            {
                Console.Write("info is incorrect ");
            }
            Console.WriteLine("ok");

            contactdb.shutdown();

            
            return 0;
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args, "config.client");
    }
}
