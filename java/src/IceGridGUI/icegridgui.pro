# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

# Preserve all public applications.

-keepclasseswithmembers public class * {
    public static void main(java.lang.String[]);
}

# Following options are useful for debugging. 
# -printseeds
# -verbose

# Preserve all annotations.

-keepattributes *Annotation*

# Parse non-public libraries to prevent undefined symbol errors on some platforms.

-dontskipnonpubliclibraryclasses

# Preserve all native method names and the names of their classes.

-keepclasseswithmembernames class * {
    native <methods>;
}

# Preserve a method that is required in all enumeration classes.

-keepclassmembers class * extends java.lang.Enum {
    public **[] values();
}

-dontnote IceInternal.*ObserverI*
-dontnote IceInternal.BasicStream
-dontnote IceInternal.Network

-dontnote IceBox.ServiceManagerI
-dontnote IceGridGUI.Coordinator
-dontnote IceGridGUI.LiveDeployment.MetricsView*

-dontnote com.jgoodies.**
-dontnote com.javafx.**

-dontusemixedcaseclassnames
-dontoptimize
-dontobfuscate

-keepclassmembers class * implements java.io.Serializable {
    static final long serialVersionUID;
    private void writeObject(java.io.ObjectOutputStream);
    private void readObject(java.io.ObjectInputStream);
    java.lang.Object writeReplace();
    java.lang.Object readResolve();
}

# Your application may contain more items that need to be preserved; 
# typically classes that are dynamically created using Class.forName:

-keep,includedescriptorclasses public class Ice.** {
  public *;
}
-keep public class IceGrid.** {
  public *;
}
-keep public class IceSSL.** {
  public *;
}
-keep interface IceGrid.**
-keep class IceMX.**
-keep interface IceMX.**

-keep,includedescriptorclasses class com.jgoodies.looks.plastic.PlasticXPLookAndFeel
-keep,includedescriptorclasses class com.jgoodies.looks.plastic.PlasticFieldCaret
-keep,includedescriptorclasses class com.jgoodies.looks.windows.WindowsFieldCaret 

#
# Without this directive, the menus on Windows don't look good
#
-keepclassmembers class com.jgoodies.looks.** {
   <methods>;
}

#
# More keeps to suppress Notes
#
-keep,includedescriptorclasses public class * {
    public protected *;
}

