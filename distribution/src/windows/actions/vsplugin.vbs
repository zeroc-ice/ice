
Dim vs2008CsPath
Dim vs2008CppPath


vs2008CsPath = Session.Property("VS_2008_CS_TEMPLATE_DIR")
vs2008CppPath = Session.Property("VS_2008_CPP_TEMPLATE_DIR")


Session.Property("VS_2008_CS_TEMPLATE_DIR_MOD") = Replace(vs2008CsPath, "\.\", "\")
Session.Property("VS_2008_CPP_TEMPLATE_DIR_MOD") = Replace(vs2008CppPath, "\.\", "\")
