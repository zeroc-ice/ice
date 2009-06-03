
Dim vs2005CsPath
Dim vs2005CppPath

Dim vs2008CsPath
Dim vs2008CppPath


vs2008CsPath = Session.Property("VS_2008_CS_TEMPLATE_DIR")
vs2008CppPath = Session.Property("VS_2008_CPP_TEMPLATE_DIR")

vs2005CsPath = Session.Property("VS_2005_CS_TEMPLATE_DIR")
vs2005CppPath = Session.Property("VS_2005_CPP_TEMPLATE_DIR")


Session.Property("VS_2008_CS_TEMPLATE_DIR_MOD") = Replace(vs2008CsPath, "\.\", "\")
Session.Property("VS_2008_CPP_TEMPLATE_DIR_MOD") = Replace(vs2008CppPath, "\.\", "\")

Session.Property("VS_2005_CS_TEMPLATE_DIR_MOD") = Replace(vs2005CsPath, "\.\", "\")
Session.Property("VS_2005_CPP_TEMPLATE_DIR_MOD") = Replace(vs2005CppPath, "\.\", "\")
