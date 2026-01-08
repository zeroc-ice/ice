Option Explicit
Dim BinDir, Shell, Key, Path, Pos, Before, After

Function TrimChar(Str, Char)
    Dim Pos
    Pos = InStr(LCase(Str), LCase(Char))
    While Pos = 1
        Str = Mid(Str, Len(Char) + 1)
        Pos = InStr(LCase(Str), LCase(Char))
    Wend

    Pos = InStrRev(LCase(Str), LCase(Char))
    While Pos = Len(Str) And Len(Str) > 0
        Str = Mid(Str, 1, Len(Str) - Len(Char))
        Pos = InStrRev(LCase(Str), LCase(Char))
    Wend
    TrimChar = Str
End Function

BinDir = Session.Property("CustomActionData")

Set Shell = CreateObject("WScript.Shell")
Key = "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment\Path"
Path = Shell.RegRead(Key)

If Path <> "" Then
    Pos = InStr(LCase(Path), LCase(BinDir))
    While Pos <> 0
        If Pos > 1 Then
            Before = TrimChar(Mid(Path, 1, Pos - 1), ";")
        End If
        After = TrimChar(Mid(Path, Pos + Len(BinDir)), ";")

        If Len(Before) > 0 And Len(After) > 0 Then
            Path = Before & ";" & After
        Else
            Path = Before & After
        End If
        Pos = InStr(LCase(Path), LCase(BinDir))
    Wend
    Shell.RegWrite Key, Path, "REG_EXPAND_SZ"
End If
