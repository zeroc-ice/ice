' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports InvokeDemo

Module InvokeC

    Class Client
        Inherits Ice.Application

        Private Sub menu()
            Console.WriteLine("usage:")
            Console.WriteLine("1: print string")
            Console.WriteLine("2: print string sequence")
            Console.WriteLine("3: print dictionary")
            Console.WriteLine("4: print enum")
            Console.WriteLine("5: print struct")
            Console.WriteLine("6: print struct sequence")
            Console.WriteLine("7: print class")
            Console.WriteLine("8: get values")
            Console.WriteLine("9: throw exception")
            Console.WriteLine("s: shutdown server")
            Console.WriteLine("x: exit")
            Console.WriteLine("?: help")
        End Sub

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim obj As Ice.ObjectPrx = communicator().propertyToProxy("Printer.Proxy")

            menu()

            Dim line As String = Nothing
            Do
                Try
                    Console.Write("==> ")
                    Console.Out.Flush()
                    line = Console.In.ReadLine()
                    If line Is Nothing Then
                        Exit Do
                    End If

                    Dim outParams As Byte() = Nothing

                    If line.Equals("1") Then
                        '
                        ' Marshal the in parameter.
                        '
                        Dim outStream As Ice.OutputStream = Ice.Util.createOutputStream(communicator)
                        outStream.writeString("The streaming API works!")

                        '
                        ' Invoke operation.
                        '
                        If Not obj.ice_invoke("printString", Ice.OperationMode.Normal, outStream.finished(), outParams) Then
                            Console.Error.WriteLine("Unknown user exception")
                        End If

                        outStream.destroy()
                    ElseIf line.Equals("2") Then
                        '
                        ' Marshal the in parameter.
                        '
                        Dim outStream As Ice.OutputStream = Ice.Util.createOutputStream(communicator)
                        Dim arr As String() = {"The", "streaming", "API", "works!"}
                        StringSeqHelper.write(outStream, arr)

                        '
                        ' Invoke operation.
                        '
                        If Not obj.ice_invoke("printStringSequence", Ice.OperationMode.Normal, outStream.finished(), outParams) Then
                            Console.Error.WriteLine("Unknown user exception")
                        End If

                        outStream.destroy()
                    ElseIf line.Equals("3") Then
                        '
                        ' Marshal the in parameter.
                        '
                        Dim outStream As Ice.OutputStream = Ice.Util.createOutputStream(communicator)
                        Dim dict As StringDict = New StringDict
                        dict("The") = "streaming"
                        dict("API") = "works!"
                        StringDictHelper.write(outStream, dict)

                        '
                        ' Invoke operation.
                        '
                        If Not obj.ice_invoke("printDictionary", Ice.OperationMode.Normal, outStream.finished(), outParams) Then
                            Console.Error.WriteLine("Unknown user exception")
                        End If

                        outStream.destroy()
                    ElseIf line.Equals("4") Then
                        '
                        ' Marshal the in parameter.
                        '
                        Dim outStream As Ice.OutputStream = Ice.Util.createOutputStream(communicator)
                        ColorHelper.write(outStream, Color.green)

                        '
                        ' Invoke operation.
                        '
                        If Not obj.ice_invoke("printEnum", Ice.OperationMode.Normal, outStream.finished(), outParams) Then
                            Console.Error.WriteLine("Unknown user exception")
                        End If

                        outStream.destroy()
                    ElseIf line.Equals("5") Then
                        '
                        ' Marshal the in parameter.
                        '
                        Dim outStream As Ice.OutputStream = Ice.Util.createOutputStream(communicator)
                        Dim s As [Structure] = New [Structure]
                        s.name = "red"
                        s.value = Color.red
                        s.ice_write(outStream)

                        '
                        ' Invoke operation.
                        '
                        If Not obj.ice_invoke("printStruct", Ice.OperationMode.Normal, outStream.finished(), outParams) Then
                            Console.Error.WriteLine("Unknown user exception")
                        End If

                        outStream.destroy()
                    ElseIf line.Equals("6") Then
                        '
                        ' Marshal the in parameter.
                        '
                        Dim outStream As Ice.OutputStream = Ice.Util.createOutputStream(communicator)
                        Dim arr As [Structure]() = New [Structure](2) {}
                        arr(0) = New [Structure]
                        arr(0).name = "red"
                        arr(0).value = Color.red
                        arr(1) = New [Structure]
                        arr(1).name = "green"
                        arr(1).value = Color.green
                        arr(2) = New [Structure]
                        arr(2).name = "blue"
                        arr(2).value = Color.blue
                        StructureSeqHelper.write(outStream, arr)

                        '
                        ' Invoke operation.
                        '
                        If Not obj.ice_invoke("printStructSequence", Ice.OperationMode.Normal, outStream.finished(), _
                                              outParams) Then
                            Console.Error.WriteLine("Unknown user exception")
                        End If

                        outStream.destroy()
                    ElseIf line.Equals("7") Then
                        '
                        ' Marshal the in parameter.
                        '
                        Dim outStream As Ice.OutputStream = Ice.Util.createOutputStream(communicator)
                        Dim C As InvokeDemo.C = New InvokeDemo.C
                        C.s = New [Structure]
                        C.s.name = "blue"
                        C.s.value = Color.blue
                        CHelper.write(outStream, C)
                        outStream.writePendingObjects()

                        '
                        ' Invoke operation.
                        '
                        If Not obj.ice_invoke("printClass", Ice.OperationMode.Normal, outStream.finished(), outParams) Then
                            Console.Error.WriteLine("Unknown user exception")
                        End If

                        outStream.destroy()
                    ElseIf line.Equals("8") Then
                        '
                        ' Invoke operation.
                        '
                        If Not obj.ice_invoke("getValues", Ice.OperationMode.Normal, Nothing, outParams) Then
                            Console.Error.WriteLine("Unknown user exception")
                            Exit Try
                        End If

                        '
                        ' Unmarshal the results.
                        '
                        Dim inStream As Ice.InputStream = Ice.Util.createInputStream(communicator, outParams)
                        Dim ch As CHelper = New CHelper(inStream)
                        ch.read()
                        Dim str As String = inStream.readString()
                        inStream.readPendingObjects()
                        inStream.destroy()
                        Dim C As InvokeDemo.C = ch.value
                        Console.Error.WriteLine("Got string `" & str & "' and class: s.name=" & C.s.name & _
                                                ", s.value=" & C.s.value)
                    ElseIf line.Equals("9") Then
                        '
                        ' Invoke operation.
                        '
                        If obj.ice_invoke("throwPrintFailure", Ice.OperationMode.Normal, Nothing, outParams) Then
                            Console.Error.WriteLine("Expected exception")
                            Exit Try
                        End If

                        Dim inStream As Ice.InputStream = Ice.Util.createInputStream(communicator, outParams)
                        Try
                            inStream.throwException()
                        Catch ex As PrintFailure
                            ' Expected.
                        Catch ex As Ice.UserException
                            Console.Error.WriteLine("Unknown user exception", ex)
                        End Try
                        inStream.destroy()
                    ElseIf line.Equals("s") Then
                        obj.ice_invoke("shutdown", Ice.OperationMode.Normal, Nothing, outParams)
                    ElseIf line.Equals("x") Then
                        ' Nothing to do.
                    ElseIf line.Equals("?") Then
                        menu()
                    Else
                        Console.Error.WriteLine("unknown command `" & line & "'")
                        menu()
                    End If
                Catch ex As Ice.LocalException
                    Console.Error.WriteLine(ex)
                End Try
            Loop While Not line.Equals("x")

            Return 0
        End Function
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Client = New Client
        Dim status As Integer = app.Main(args, "config.client")
        System.Environment.Exit(status)
    End Sub

End Module
