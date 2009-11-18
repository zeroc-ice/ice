// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Reflection;
using Employees;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("EmployeesServer")]
[assembly: AssemblyDescription("Excel employees demo server")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server
{
    private class EmployeeI : EmployeeDisp_
    {
        public EmployeeI(double salary, EmployeeDetails details)
        {
            details_ = (EmployeeDetails)details.Clone();
        }

        public override EmployeeDetails getDetails(Ice.Current current__)
        {
            return details_;
        }

        public override void setAnnualSalary(double salary, Ice.Current current__)
        {
            Console.WriteLine("Setting salary for " + details_.name + ": " + salary);
        }

        public override void setTaxPayable(double amount, Ice.Current current__)
        {
            Console.WriteLine("Setting tax payable for " + details_.name + ": " + amount);
        }

        private EmployeeDetails details_;
    }

    private class EmployeeFinderI : EmployeeFinderDisp_
    {
        public override List<EmployeePrx> list(Ice.Current current__)
        {
            return employees_;
        }

        public void addEmployee(Ice.ObjectAdapter adapter, double salary, EmployeeDetails details)
        {
            EmployeeI emp = new EmployeeI(salary, details);
            Ice.ObjectPrx prx = adapter.add(emp, Ice.Util.stringToIdentity(details.name));
            EmployeePrx tmp = EmployeePrxHelper.uncheckedCast(prx);
            employees_.Add(tmp);
        }

        private List<EmployeePrx> employees_ = new List<EmployeePrx>();
    }

    class App : Ice.Application
    {
        public override int run(string[] args)
        {
            if(args.Length > 0)
            {
                System.Console.Error.WriteLine(appName() + ": too many arguments");
                return 1;
            }

            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Employees");
            EmployeeFinderI finder = new EmployeeFinderI();
            adapter.add(finder, communicator().stringToIdentity("Finder"));

            EmployeeDetails details = new EmployeeDetails();

            details.name = "Fred Halloway";
            details.address = "25 Bond St";
            finder.addEmployee(adapter, 42750, details);

            details.name = "Joseph Waterton";
            details.address = "17 Merivale St";
            finder.addEmployee(adapter, 78220, details);

            details.name = "Broughton Syverson";
            details.address = "20 Appel St";
            finder.addEmployee(adapter, 124000, details);
            
            adapter.activate();
            communicator().waitForShutdown();
            return 0;
        }
    }

    public static void Main(string[] args)
    {
        App app = new App();
        int status = app.main(args, "config.server");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
