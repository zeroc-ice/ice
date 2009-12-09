// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module Employees
{
    struct EmployeeDetails
    {
        string name;
	string address;
	// ...
    };

    interface Employee
    {
	EmployeeDetails getDetails();
        void setAnnualSalary(double salary);
	void setTaxPayable(double amount);
    };

    ["clr:generic:List"] sequence<Employee*> EmployeeSeq;

    interface EmployeeFinder
    {
        EmployeeSeq list();
	// ...
    };
};
