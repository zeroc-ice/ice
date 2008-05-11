Imports Microsoft.VisualBasic
Imports System
Imports FileSystem

Module Client

    ' Recursively print the contents of directory "dir"
    ' in tree fashion. For files, show the contents of
    ' each file. The "depth" parameter is the current
    ' nesting level (for indentation).

    Sub listRecursive(ByVal dir As DirectoryPrx, ByVal depth As Integer)
        depth += 1
        Dim indent As String = New String(Chr(9), depth)

        Dim contents As NodePrx() = dir.list()

        For Each node As NodePrx In contents
            Dim subdir As DirectoryPrx = DirectoryPrxHelper.checkedCast(node)
            Dim file As FilePrx = FilePrxHelper.uncheckedCast(node)
            Console.Write(indent & node.name())
            If Not subdir Is Nothing Then
                Console.WriteLine(" (directory):")
            Else
                Console.WriteLine(" (file):")
            End If
            If Not subdir Is Nothing Then
                listRecursive(subdir, depth)
            Else
                Dim text As String() = file.read()
                For j As Integer = 0 To text.Length - 1
                    Console.WriteLine(indent & "    " & text(j))
                Next
            End If
        Next
    End Sub

    Public Sub Main(ByVal args() As String)
        Dim status As Integer = 0
        Dim ic As Ice.Communicator = Nothing
        Try
            ' Create a communicator
            '
            ic = Ice.Util.initialize(args)

            ' Create a proxy for the root directory
            '
            Dim obj As Ice.ObjectPrx = ic.stringToProxy("RootDir:default -p 10000")

            ' Down-cast the proxy to a Directory proxy
            '
            Dim rootDir As DirectoryPrx = DirectoryPrxHelper.checkedCast(obj)

            ' Recursively list the contents of the root directory
            '
            Console.WriteLine("Contents of root directory:")
            listRecursive(rootDir, 0)
        Catch e As Exception
            Console.Error.WriteLine(e)
            status = 1
        End Try
        If Not ic Is Nothing Then
            ' Clean up
            '
            Try
                ic.destroy()
            Catch e As Exception
                Console.Error.WriteLine(e)
                status = 1
            End Try
        End If
        Environment.Exit(status)
    End Sub

End Module
