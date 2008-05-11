Imports System
Imports FileSystem

Module Server

    Public Class Server
        Inherits Ice.Application

        Public Overrides Function run(ByVal args As String()) As Integer
            ' Terminate cleanly on receipt of a signal
            '
            shutdownOnInterrupt()

            ' Create an object adapter (stored in the _adapter
            ' static members)
            '
            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapterWithEndpoints( _
                     "SimpleFilesystem", "default -p 10000")
            DirectoryI._adapter = adapter
            FileI._adapter = adapter

            ' Create the root directory (with name "/" and no parent)
            '
            Dim root As DirectoryI = New DirectoryI("/", Nothing)

            ' Create a file called "README" in the root directory
            '
            Dim file As FileI = New FileI("README", root)
            Dim text As String() = New String() {"This file system contains a collection of poetry."}
            Try
                file.write(text)
            Catch e As GenericError
                Console.Error.WriteLine(e.reason)
            End Try

            ' Create a directory called "Coleridge"
            ' in the root directory
            '
            Dim coleridge As DirectoryI = New DirectoryI("Coleridge", root)

            ' Create a file called "Kubla_Khan"
            ' in the Coleridge directory
            '
            file = New FileI("Kubla_Khan", coleridge)
            text = New String() {"In Xanadu did Kubla Khan", _
            "A stately pleasure-dome decree:", _
            "Where Alph, the sacred river, ran", _
            "Through caverns measureless to man", _
            "Down to a sunless sea."}
            Try
                CType(file, FileOperationsNC_).write(text)
                file.write(text)
            Catch e As GenericError
                Console.Error.WriteLine(e.reason)
            End Try

            ' All objects are created, allow client requests now
            '
            adapter.activate()

            ' Wait until we are done
            '
            communicator().waitForShutdown()

            If interrupted() Then
                Console.Error.WriteLine(appName() & ": terminating")
            End If

            Return 0
        End Function

    End Class

    Public Sub Main(ByVal args As String())
        Dim app As Server = New Server
        Environment.Exit(app.Main(args))
    End Sub

End Module
