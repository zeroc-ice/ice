' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports System.Diagnostics
Imports Demo

Module ValueC
    Class Client
        Inherits Ice.Application

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim initial As InitialPrx = InitialPrxHelper.checkedCast(communicator().propertyToProxy("Value.Initial"))
            If initial Is Nothing Then
                Console.Error.WriteLine("invalid object reference")
                Return 1
            End If

            Console.Out.WriteLine()
            Console.Out.WriteLine("Let's first transfer a simple object, for a class without")
            Console.Out.WriteLine("operations, and print its contents. No factory is required")
            Console.Out.WriteLine("for this.")
            Console.Out.WriteLine("[press enter]")
            Console.In.ReadLine()

            Dim simple As simple = initial.getSimple()
            Console.Out.WriteLine("==> " & simple.message)

            Console.Out.WriteLine()
            Console.Out.WriteLine("Yes, this worked. Now let's try to transfer an object for a class")
            Console.Out.WriteLine("with operations as type Demo.Printer, without installing a factory first.")
            Console.Out.WriteLine("This should give us a `no factory' exception.")
            Console.Out.WriteLine("[press enter]")
            Console.In.ReadLine()

            Dim printer As printer = Nothing
            Dim printerProxy As PrinterPrx = Nothing
            Try
                initial.getPrinter(printer, printerProxy)
                Console.Error.WriteLine("Did not get the expected NoObjectFactoryException!")
                Environment.Exit(1)
            Catch ex As Ice.NoObjectFactoryException
                Console.Out.WriteLine("==> " & ex.ToString())
            End Try

            Console.Out.WriteLine()
            Console.Out.WriteLine("Yep, that's what we expected. Now let's try again, but with")
            Console.Out.WriteLine("installing an appropriate factory first. If successful, we print")
            Console.Out.WriteLine("the object's content.")
            Console.Out.WriteLine("[press enter]")
            Console.In.ReadLine()

            Dim factory As Ice.ObjectFactory = New ObjectFactory
            communicator().addObjectFactory(factory, "::Demo::Printer")

            initial.getPrinter(printer, printerProxy)
            Console.Out.WriteLine("==> " & printer.message)

            Console.Out.WriteLine()
            Console.Out.WriteLine("Cool, it worked! Let's try calling the printBackwards() method")
            Console.Out.WriteLine("on the object we just received locally.")
            Console.Out.WriteLine("[press enter]")
            Console.In.ReadLine()

            Console.Out.Write("==> ")
            printer.printBackwards()

            Console.Out.WriteLine()
            Console.Out.WriteLine("Now we call the same method, but on the remote object. Watch the")
            Console.Out.WriteLine("server's output.")
            Console.Out.WriteLine("[press enter]")
            Console.In.ReadLine()

            printerProxy.printBackwards()

            Console.Out.WriteLine()
            Console.Out.WriteLine("Next, we transfer a derived object from the server as a base")
            Console.Out.WriteLine("object. Since we haven't yet installed a factory for the derived")
            Console.Out.WriteLine("class, the derived class (Demo.DerivedPrinter) is sliced")
            Console.Out.WriteLine("to its base class (Demo.Printer).")
            Console.Out.WriteLine("[press enter]")
            Console.In.ReadLine()

            Dim derivedAsBase As printer = initial.getDerivedPrinter()
            Console.Out.WriteLine("The type ID of the received object is """ & derivedAsBase.ice_id() & """")
            Debug.Assert(derivedAsBase.ice_id().Equals("::Demo::Printer"))

            Console.Out.WriteLine()
            Console.Out.WriteLine("Now we install a factory for the derived class, and try again.")
            Console.Out.WriteLine("Because we receive the derived object as a base object,")
            Console.Out.WriteLine("we need to do a class cast to get from the base to the derived object.")
            Console.Out.WriteLine("[press enter]")
            Console.In.ReadLine()

            communicator().addObjectFactory(factory, "::Demo::DerivedPrinter")

            derivedAsBase = initial.getDerivedPrinter()
            Dim derived As DerivedPrinter = CType(derivedAsBase, DerivedPrinter)

            Console.Out.WriteLine("==> class cast to derived object succeded")
            Console.Out.WriteLine("The type ID of the received object is """ & derived.ice_id() & """")

            Console.Out.WriteLine()
            Console.Out.WriteLine("Let's print the message contained in the derived object, and")
            Console.Out.WriteLine("call the operation printUppercase() on the derived object")
            Console.Out.WriteLine("locally.")
            Console.Out.WriteLine("[press enter]")
            Console.In.ReadLine()

            Console.Out.WriteLine("==> " & derived.derivedMessage)
            Console.Out.Write("==> ")
            derived.printUppercase()

            Console.Out.WriteLine()
            Console.Out.WriteLine("Finally, we try the same again, but instead of returning the")
            Console.Out.WriteLine("derived object, we throw an exception containing the derived")
            Console.Out.WriteLine("object.")
            Console.Out.WriteLine("[press enter]")
            Console.In.ReadLine()

            Try
                initial.throwDerivedPrinter()
            Catch ex As DerivedPrinterException
                derived = ex.derived
                Debug.Assert(Not derived Is Nothing)
            End Try

            Console.Out.WriteLine("==> " & derived.derivedMessage)
            Console.Out.Write("==> ")
            derived.printUppercase()

            Console.Out.WriteLine()
            Console.Out.WriteLine("That's it for this demo. Have fun with Ice!")

            initial.shutdown()

            Return 0
        End Function

    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Client = New Client
        Dim status As Integer = app.main(args, "config.client")
        System.Environment.Exit(status)
    End Sub
End Module
