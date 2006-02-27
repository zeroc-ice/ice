#----------------------------------------------------------------------------
#
# Makefile,v 1.61 2003/11/07 06:45:30 dhinton Exp
#
#----------------------------------------------------------------------------

export ACE_ROOT := $(TAO_HOME)
export TAO_ROOT := $(TAO_HOME)/TAO

#----------------------------------------------------------------------------
#       Local macros
#----------------------------------------------------------------------------

debug=0

BIN2 =  Service \
	Consumer \
	Supplier \
	Destroyer \
	Notifier

IDL_FILES = Perf
IDL_SRC = PerfC.cpp PerfS.cpp

PSRC= $(IDL_SRC) $(addsuffix .cpp,$(BIN2))

SUPPLIER_OBJS = \
		Worker_Thread.o \
                PerfC.o \
                PerfS.o \
                Supplier.o

CONSUMER_OBJS = \
		Worker_Thread.o \
                PerfC.o \
                PerfS.o \
                Consumer.o

SERVICE_OBJS = \
		Worker_Thread.o \
		Service.o

DESTROYER_OBJS = \
	         Destroyer.o

NOTIFIER_OBJS = \
	         Notifier.o \
	         PerfC.o \
                 PerfS.o

LDLIBS = -lTAO_CosEvent -lTAO_CosNaming -lTAO_Strategies -lTAO_Svc_Utils -lTAO_IORTable -lTAO_Messaging $(TAO_SRVR_LIBS)
CPPFLAGS += -I$(TAO_ROOT) -I$(TAO_ROOT)/orbsvcs 

#----------------------------------------------------------------------------
#       Include macros and targets
#----------------------------------------------------------------------------

include $(ACE_ROOT)/include/makeinclude/wrapper_macros.GNU
include $(ACE_ROOT)/include/makeinclude/macros.GNU
include $(TAO_ROOT)/rules.tao.GNU

#### If the orbsvcs library wasn't built with all components, don't
#### try to build certain tests.
TAO_ORBSVCS := $(shell sh $(ACE_ROOT)/bin/ace_components --orbsvcs)
ifeq (NativeCosEvent,$(findstring NativeCosEvent,$(TAO_ORBSVCS)))
  BIN = $(BIN2)
endif # NativeCosEvent

include $(ACE_ROOT)/include/makeinclude/rules.common.GNU
include $(ACE_ROOT)/include/makeinclude/rules.nonested.GNU
include $(ACE_ROOT)/include/makeinclude/rules.local.GNU

# To build multiple executables in the same directory on AIX, it works
# best to wipe out any previously-created tempinc directory.
# The compiler/linker isn't too smart about instantiating templates...
ifdef TEMPINCDIR
COMPILE.cc := $(RM) -rf tempinc; $(COMPILE.cc)
endif

#----------------------------------------------------------------------------
#       Local targets
#----------------------------------------------------------------------------

.PRECIOUS: PerfC.cpp PerfC.i PerfC.h
.PRECIOUS: PerfS.cpp PerfS.i PerfS.h
.PRECIOUS: PerfS_T.cpp PerfS_T.i PerfS_T.h

Supplier: $(addprefix $(VDIR),$(SUPPLIER_OBJS))
	$(LINK.cc) $(LDFLAGS) -o $@ $^ $(VLDLIBS) $(POSTLINK)

Consumer: $(addprefix $(VDIR),$(CONSUMER_OBJS))
	$(LINK.cc) $(LDFLAGS) -o $@ $^ $(VLDLIBS) $(POSTLINK)

Service: $(addprefix $(VDIR),$(SERVICE_OBJS))
	$(LINK.cc) $(LDFLAGS) -o $@ $^ $(VLDLIBS) $(POSTLINK)

Destroyer: $(addprefix $(VDIR),$(DESTROYER_OBJS))
	$(LINK.cc) $(LDFLAGS) -o $@ $^ $(VLDLIBS) $(POSTLINK)

Notifier: $(addprefix $(VDIR),$(NOTIFIER_OBJS))
	$(LINK.cc) $(LDFLAGS) -o $@ $^ $(VLDLIBS) $(POSTLINK)

realclean: clean
	-$(RM) -rf PerfC.* PerfS.* PerfS_T.*

#----------------------------------------------------------------------------
#       Dependencies
#----------------------------------------------------------------------------
# DO NOT DELETE THIS LINE -- g++dep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.


.obj/Service.o .obj/Service.so .shobj/Service.o .shobj/Service.so: Service.cpp \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEvent/CEC_EventChannel.h \
  $(ACE_ROOT)/ace/pre.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEvent/CEC_Factory.h \
  $(ACE_ROOT)/ace/Service_Object.h \
  $(ACE_ROOT)/ace/Shared_Object.h \
  $(ACE_ROOT)/ace/ACE_export.h \
  $(ACE_ROOT)/ace/post.h \
  $(ACE_ROOT)/ace/ace_wchar.h \
  $(ACE_ROOT)/ace/ace_wchar.inl \
  $(ACE_ROOT)/ace/OS_main.h \
  $(ACE_ROOT)/ace/os_include/sys/os_types.h \
  $(ACE_ROOT)/ace/os_include/os_stddef.h \
  $(ACE_ROOT)/ace/Svc_Conf_Tokens.h \
  $(ACE_ROOT)/ace/Event_Handler.h \
  $(ACE_ROOT)/ace/os_include/os_signal.h \
  $(ACE_ROOT)/ace/os_include/os_time.h \
  $(ACE_ROOT)/ace/os_include/os_ucontext.h \
  $(ACE_ROOT)/ace/Atomic_Op.h \
  $(ACE_ROOT)/ace/Thread_Mutex.h \
  $(ACE_ROOT)/ace/OS_NS_Thread.h \
  $(ACE_ROOT)/ace/Global_Macros.h \
  $(ACE_ROOT)/ace/OS_Errno.h \
  $(ACE_ROOT)/ace/os_include/os_errno.h \
  $(ACE_ROOT)/ace/Basic_Types.h \
  $(ACE_ROOT)/ace/os_include/os_limits.h \
  $(ACE_ROOT)/ace/os_include/os_unistd.h \
  $(ACE_ROOT)/ace/os_include/os_inttypes.h \
  $(ACE_ROOT)/ace/os_include/os_stdint.h \
  $(ACE_ROOT)/ace/os_include/os_stdio.h \
  $(ACE_ROOT)/ace/os_include/os_stdarg.h \
  $(ACE_ROOT)/ace/os_include/os_float.h \
  $(ACE_ROOT)/ace/os_include/os_stdlib.h \
  $(ACE_ROOT)/ace/os_include/sys/os_wait.h \
  $(ACE_ROOT)/ace/os_include/sys/os_resource.h \
  $(ACE_ROOT)/ace/os_include/sys/os_time.h \
  $(ACE_ROOT)/ace/Default_Constants.h \
  $(ACE_ROOT)/ace/os_include/os_pthread.h \
  $(ACE_ROOT)/ace/Base_Thread_Adapter.h \
  $(ACE_ROOT)/ace/OS_Log_Msg_Attributes.h \
  $(ACE_ROOT)/ace/iosfwd.h \
  $(ACE_ROOT)/ace/os_include/sys/os_sem.h \
  $(ACE_ROOT)/ace/os_include/sys/os_ipc.h \
  $(ACE_ROOT)/ace/os_include/os_semaphore.h \
  $(ACE_ROOT)/ace/OS_Memory.h \
  $(ACE_ROOT)/ace/OS_NS_stdlib.h \
  $(ACE_ROOT)/ace/OS_NS_signal.h \
  $(ACE_ROOT)/ace/Atomic_Op_T.h \
  $(ACE_ROOT)/ace/Atomic_Op_T.cpp \
  $(ACE_ROOT)/ace/Log_Msg.h \
  $(ACE_ROOT)/ace/Log_Priority.h \
  $(ACE_ROOT)/ace/Atomic_Op_T.i \
  $(ACE_ROOT)/ace/Guard_T.h \
  $(ACE_ROOT)/ace/Lock.h \
  $(ACE_ROOT)/ace/Guard_T.cpp \
  $(ACE_ROOT)/ace/Guard_T.inl \
  $(ACE_ROOT)/ace/RW_Thread_Mutex.h \
  $(ACE_ROOT)/ace/RW_Mutex.h \
  $(ACE_ROOT)/ace/Synch_Traits.h \
  $(ACE_ROOT)/ace/DLL.h \
  $(ACE_ROOT)/ace/os_include/os_dlfcn.h \
  $(TAO_ROOT)/tao/Objref_VarOut_T.h \
  $(ACE_ROOT)/ace/CORBA_macros.h \
  $(ACE_ROOT)/ace/Exception_Macros.h \
  $(TAO_ROOT)/tao/Basic_Types.h \
  $(ACE_ROOT)/ace/CDR_Base.h \
  $(TAO_ROOT)/tao/varbase.h \
  $(TAO_ROOT)/tao/Objref_VarOut_T.cpp \
  $(TAO_ROOT)/tao/Environment.h \
  $(TAO_ROOT)/tao/TAO_Export.h \
  $(TAO_ROOT)/tao/CORBA_methods.h \
  $(TAO_ROOT)/tao/orbconf.h \
  $(TAO_ROOT)/tao/Pseudo_VarOut_T.h \
  $(TAO_ROOT)/tao/Pseudo_VarOut_T.cpp \
  $(TAO_ROOT)/tao/Pseudo_VarOut_T.inl \
  $(TAO_ROOT)/tao/default_environment.h \
  $(TAO_ROOT)/tao/Objref_VarOut_T.inl \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEvent/event_export.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventChannelAdminS.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventCommS.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventCommC.h \
  $(TAO_ROOT)/tao/ORB.h \
  $(TAO_ROOT)/tao/Exception.h \
  $(ACE_ROOT)/ace/SStringfwd.h \
  $(TAO_ROOT)/tao/objectid.h \
  $(TAO_ROOT)/tao/PolicyC.h \
  $(TAO_ROOT)/tao/CurrentC.h \
  $(TAO_ROOT)/tao/Object.h \
  $(TAO_ROOT)/tao/IOP_IORC.h \
  $(TAO_ROOT)/tao/OctetSeqC.h \
  $(TAO_ROOT)/tao/Sequence.h \
  $(TAO_ROOT)/tao/Managed_Types.h \
  $(TAO_ROOT)/tao/CORBA_String.h \
  $(TAO_ROOT)/tao/Sequence_T.h \
  $(TAO_ROOT)/tao/Sequence_T.cpp \
  $(TAO_ROOT)/tao/Array_VarOut_T.h \
  $(TAO_ROOT)/tao/Array_VarOut_T.cpp \
  $(TAO_ROOT)/tao/Array_VarOut_T.inl \
  $(TAO_ROOT)/tao/Sequence_T.i \
  $(TAO_ROOT)/tao/Seq_Var_T.h \
  $(TAO_ROOT)/tao/Seq_Var_T.cpp \
  $(TAO_ROOT)/tao/Seq_Var_T.inl \
  $(TAO_ROOT)/tao/Seq_Out_T.h \
  $(TAO_ROOT)/tao/Seq_Out_T.cpp \
  $(TAO_ROOT)/tao/Seq_Out_T.inl \
  $(TAO_ROOT)/tao/CDR.h \
  $(ACE_ROOT)/ace/CDR_Stream.h \
  $(ACE_ROOT)/ace/Message_Block.h \
  $(ACE_ROOT)/ace/Time_Value.h \
  $(ACE_ROOT)/ace/Message_Block_T.h \
  $(ACE_ROOT)/ace/Message_Block_T.cpp \
  $(ACE_ROOT)/ace/Malloc_Base.h \
  $(ACE_ROOT)/ace/os_include/sys/os_mman.h \
  $(ACE_ROOT)/ace/Message_Block_T.i \
  $(TAO_ROOT)/tao/VarOut_T.h \
  $(TAO_ROOT)/tao/VarOut_T.cpp \
  $(TAO_ROOT)/tao/VarOut_T.inl \
  $(TAO_ROOT)/tao/Policy_ForwardC.h \
  $(TAO_ROOT)/tao/Object_Argument_T.h \
  $(TAO_ROOT)/tao/Argument.h \
  $(TAO_ROOT)/tao/Object_Argument_T.cpp \
  $(TAO_ROOT)/tao/Dynamic_ParameterC.h \
  $(TAO_ROOT)/tao/ParameterMode.h \
  $(TAO_ROOT)/tao/Any.h \
  $(TAO_ROOT)/tao/Arg_Traits_T.h \
  $(TAO_ROOT)/tao/Object_Argument_T.inl \
  $(TAO_ROOT)/tao/OBV_Constants.h \
  $(TAO_ROOT)/tao/Collocation_Proxy_Broker.h \
  $(TAO_ROOT)/tao/Collocation_Strategy.h \
  $(TAO_ROOT)/tao/PortableServer/PortableServer.h \
  $(TAO_ROOT)/tao/PortableServer/portableserver_export.h \
  $(TAO_ROOT)/tao/PortableServer/PortableServerC.h \
  $(TAO_ROOT)/tao/PortableServer/Servant_Base.h \
  $(TAO_ROOT)/tao/Abstract_Servant_Base.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventCommS_T.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventCommS_T.cpp \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventCommS_T.i \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventChannelAdminC.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventChannelAdminS_T.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventChannelAdminS_T.cpp \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventChannelAdminS_T.i \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEvent/CEC_Defaults.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEvent/CEC_Default_Factory.h \
  $(ACE_ROOT)/ace/Service_Config.h \
  $(ACE_ROOT)/ace/Unbounded_Queue.h \
  $(ACE_ROOT)/ace/Node.h \
  $(ACE_ROOT)/ace/Node.cpp \
  $(ACE_ROOT)/ace/Unbounded_Queue.cpp \
  $(ACE_ROOT)/ace/Unbounded_Queue.inl \
  $(ACE_ROOT)/ace/Unbounded_Set.h \
  $(ACE_ROOT)/ace/Unbounded_Set.cpp \
  $(ACE_ROOT)/ace/Unbounded_Set.inl \
  $(ACE_ROOT)/ace/SString.h \
  $(ACE_ROOT)/ace/String_Base.h \
  $(ACE_ROOT)/ace/String_Base_Const.h \
  $(ACE_ROOT)/ace/String_Base.cpp \
  $(ACE_ROOT)/ace/ACE.h \
  $(ACE_ROOT)/ace/Flag_Manip.h \
  $(ACE_ROOT)/ace/Flag_Manip.i \
  $(ACE_ROOT)/ace/OS_NS_fcntl.h \
  $(ACE_ROOT)/ace/os_include/os_fcntl.h \
  $(ACE_ROOT)/ace/os_include/sys/os_stat.h \
  $(ACE_ROOT)/ace/Handle_Ops.h \
  $(ACE_ROOT)/ace/Handle_Ops.i \
  $(ACE_ROOT)/ace/Lib_Find.h \
  $(ACE_ROOT)/ace/Lib_Find.i \
  $(ACE_ROOT)/ace/Init_ACE.h \
  $(ACE_ROOT)/ace/Init_ACE.i \
  $(ACE_ROOT)/ace/Sock_Connect.h \
  $(ACE_ROOT)/ace/os_include/netinet/os_in.h \
  $(ACE_ROOT)/ace/os_include/sys/os_socket.h \
  $(ACE_ROOT)/ace/os_include/sys/os_uio.h \
  $(ACE_ROOT)/ace/Sock_Connect.i \
  $(ACE_ROOT)/ace/ACE.i \
  $(ACE_ROOT)/ace/OS_NS_unistd.h \
  $(ACE_ROOT)/ace/os_include/os_ctype.h \
  $(ACE_ROOT)/ace/OS_NS_sys_socket.h \
  $(ACE_ROOT)/ace/os_include/net/os_if.h \
  $(ACE_ROOT)/ace/OS_NS_stropts.h \
  $(ACE_ROOT)/ace/os_include/os_stropts.h \
  $(ACE_ROOT)/ace/Auto_Ptr.h \
  $(ACE_ROOT)/ace/Auto_Ptr.cpp \
  $(ACE_ROOT)/ace/Auto_Ptr.i \
  $(ACE_ROOT)/ace/OS_NS_string.h \
  $(ACE_ROOT)/ace/String_Base.i \
  $(ACE_ROOT)/ace/Min_Max.h \
  $(ACE_ROOT)/ace/XML_Svc_Conf.h \
  $(ACE_ROOT)/ace/Reactor.h \
  $(ACE_ROOT)/ace/Timer_Queuefwd.h \
  $(ACE_ROOT)/ace/Reactor_Timer_Interface.h \
  $(ACE_ROOT)/ace/Get_Opt.h \
  $(ACE_ROOT)/ace/Containers.h \
  $(ACE_ROOT)/ace/Containers_T.h \
  $(ACE_ROOT)/ace/Array_Base.h \
  $(ACE_ROOT)/ace/Array_Base.cpp \
  $(ACE_ROOT)/ace/Array_Base.inl \
  $(ACE_ROOT)/ace/Containers_T.cpp \
  $(ACE_ROOT)/ace/Containers_T.i \
  $(ACE_ROOT)/ace/OS_NS_stdio.h

.obj/Consumer.o .obj/Consumer.so .shobj/Consumer.o .shobj/Consumer.so: Consumer.cpp Consumer.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventCommS.h \
  $(ACE_ROOT)/ace/pre.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventCommC.h \
  $(TAO_ROOT)/tao/ORB.h \
  $(TAO_ROOT)/tao/Exception.h \
  $(TAO_ROOT)/tao/orbconf.h \
  $(ACE_ROOT)/ace/Basic_Types.h \
  $(ACE_ROOT)/ace/post.h \
  $(ACE_ROOT)/ace/ace_wchar.h \
  $(ACE_ROOT)/ace/ace_wchar.inl \
  $(ACE_ROOT)/ace/OS_main.h \
  $(ACE_ROOT)/ace/os_include/os_limits.h \
  $(ACE_ROOT)/ace/os_include/os_unistd.h \
  $(ACE_ROOT)/ace/os_include/sys/os_types.h \
  $(ACE_ROOT)/ace/os_include/os_stddef.h \
  $(ACE_ROOT)/ace/os_include/os_inttypes.h \
  $(ACE_ROOT)/ace/os_include/os_stdint.h \
  $(ACE_ROOT)/ace/os_include/os_stdio.h \
  $(ACE_ROOT)/ace/os_include/os_stdarg.h \
  $(ACE_ROOT)/ace/os_include/os_float.h \
  $(ACE_ROOT)/ace/os_include/os_stdlib.h \
  $(ACE_ROOT)/ace/os_include/sys/os_wait.h \
  $(ACE_ROOT)/ace/os_include/os_signal.h \
  $(ACE_ROOT)/ace/os_include/os_time.h \
  $(ACE_ROOT)/ace/os_include/os_ucontext.h \
  $(ACE_ROOT)/ace/os_include/sys/os_resource.h \
  $(ACE_ROOT)/ace/os_include/sys/os_time.h \
  $(ACE_ROOT)/ace/ACE_export.h \
  $(ACE_ROOT)/ace/Global_Macros.h \
  $(ACE_ROOT)/ace/OS_Errno.h \
  $(ACE_ROOT)/ace/os_include/os_errno.h \
  $(ACE_ROOT)/ace/Synch_Traits.h \
  $(ACE_ROOT)/ace/Lock.h \
  $(ACE_ROOT)/ace/SStringfwd.h \
  $(ACE_ROOT)/ace/iosfwd.h \
  $(ACE_ROOT)/ace/CORBA_macros.h \
  $(ACE_ROOT)/ace/Exception_Macros.h \
  $(TAO_ROOT)/tao/TAO_Export.h \
  $(TAO_ROOT)/tao/Basic_Types.h \
  $(ACE_ROOT)/ace/CDR_Base.h \
  $(ACE_ROOT)/ace/Default_Constants.h \
  $(TAO_ROOT)/tao/objectid.h \
  $(TAO_ROOT)/tao/PolicyC.h \
  $(TAO_ROOT)/tao/CurrentC.h \
  $(TAO_ROOT)/tao/Object.h \
  $(TAO_ROOT)/tao/IOP_IORC.h \
  $(TAO_ROOT)/tao/OctetSeqC.h \
  $(TAO_ROOT)/tao/Sequence.h \
  $(TAO_ROOT)/tao/Managed_Types.h \
  $(TAO_ROOT)/tao/CORBA_String.h \
  $(TAO_ROOT)/tao/default_environment.h \
  $(TAO_ROOT)/tao/Sequence_T.h \
  $(TAO_ROOT)/tao/Objref_VarOut_T.h \
  $(TAO_ROOT)/tao/varbase.h \
  $(TAO_ROOT)/tao/Objref_VarOut_T.cpp \
  $(TAO_ROOT)/tao/Environment.h \
  $(TAO_ROOT)/tao/CORBA_methods.h \
  $(TAO_ROOT)/tao/Pseudo_VarOut_T.h \
  $(TAO_ROOT)/tao/Pseudo_VarOut_T.cpp \
  $(TAO_ROOT)/tao/Pseudo_VarOut_T.inl \
  $(TAO_ROOT)/tao/Objref_VarOut_T.inl \
  $(TAO_ROOT)/tao/Sequence_T.cpp \
  $(TAO_ROOT)/tao/Array_VarOut_T.h \
  $(TAO_ROOT)/tao/Array_VarOut_T.cpp \
  $(TAO_ROOT)/tao/Array_VarOut_T.inl \
  $(ACE_ROOT)/ace/OS_Memory.h \
  $(ACE_ROOT)/ace/OS_NS_stdlib.h \
  $(TAO_ROOT)/tao/Sequence_T.i \
  $(TAO_ROOT)/tao/Seq_Var_T.h \
  $(TAO_ROOT)/tao/Seq_Var_T.cpp \
  $(TAO_ROOT)/tao/Seq_Var_T.inl \
  $(TAO_ROOT)/tao/Seq_Out_T.h \
  $(TAO_ROOT)/tao/Seq_Out_T.cpp \
  $(TAO_ROOT)/tao/Seq_Out_T.inl \
  $(TAO_ROOT)/tao/CDR.h \
  $(ACE_ROOT)/ace/CDR_Stream.h \
  $(ACE_ROOT)/ace/Message_Block.h \
  $(ACE_ROOT)/ace/Time_Value.h \
  $(ACE_ROOT)/ace/Message_Block_T.h \
  $(ACE_ROOT)/ace/Message_Block_T.cpp \
  $(ACE_ROOT)/ace/Malloc_Base.h \
  $(ACE_ROOT)/ace/os_include/sys/os_mman.h \
  $(ACE_ROOT)/ace/Message_Block_T.i \
  $(TAO_ROOT)/tao/VarOut_T.h \
  $(TAO_ROOT)/tao/VarOut_T.cpp \
  $(TAO_ROOT)/tao/VarOut_T.inl \
  $(TAO_ROOT)/tao/Policy_ForwardC.h \
  $(TAO_ROOT)/tao/Object_Argument_T.h \
  $(TAO_ROOT)/tao/Argument.h \
  $(TAO_ROOT)/tao/Object_Argument_T.cpp \
  $(TAO_ROOT)/tao/Dynamic_ParameterC.h \
  $(TAO_ROOT)/tao/ParameterMode.h \
  $(TAO_ROOT)/tao/Any.h \
  $(TAO_ROOT)/tao/Arg_Traits_T.h \
  $(TAO_ROOT)/tao/Object_Argument_T.inl \
  $(TAO_ROOT)/tao/OBV_Constants.h \
  $(ACE_ROOT)/ace/Thread_Mutex.h \
  $(ACE_ROOT)/ace/OS_NS_Thread.h \
  $(ACE_ROOT)/ace/os_include/os_pthread.h \
  $(ACE_ROOT)/ace/Base_Thread_Adapter.h \
  $(ACE_ROOT)/ace/OS_Log_Msg_Attributes.h \
  $(ACE_ROOT)/ace/os_include/sys/os_sem.h \
  $(ACE_ROOT)/ace/os_include/sys/os_ipc.h \
  $(ACE_ROOT)/ace/os_include/os_semaphore.h \
  $(ACE_ROOT)/ace/OS_NS_signal.h \
  $(ACE_ROOT)/ace/Guard_T.h \
  $(ACE_ROOT)/ace/Guard_T.cpp \
  $(ACE_ROOT)/ace/Guard_T.inl \
  $(ACE_ROOT)/ace/RW_Thread_Mutex.h \
  $(ACE_ROOT)/ace/RW_Mutex.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEvent/event_export.h \
  $(TAO_ROOT)/tao/Collocation_Proxy_Broker.h \
  $(TAO_ROOT)/tao/Collocation_Strategy.h \
  $(TAO_ROOT)/tao/PortableServer/PortableServer.h \
  $(TAO_ROOT)/tao/PortableServer/portableserver_export.h \
  $(TAO_ROOT)/tao/PortableServer/PortableServerC.h \
  $(TAO_ROOT)/tao/PortableServer/Servant_Base.h \
  $(TAO_ROOT)/tao/Abstract_Servant_Base.h \
  $(ACE_ROOT)/ace/Atomic_Op.h \
  $(ACE_ROOT)/ace/Atomic_Op_T.h \
  $(ACE_ROOT)/ace/Atomic_Op_T.cpp \
  $(ACE_ROOT)/ace/Log_Msg.h \
  $(ACE_ROOT)/ace/Log_Priority.h \
  $(ACE_ROOT)/ace/Atomic_Op_T.i \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventCommS_T.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventCommS_T.cpp \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventCommS_T.i \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventChannelAdminS.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventChannelAdminC.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventChannelAdminS_T.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventChannelAdminS_T.cpp \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventChannelAdminS_T.i

.obj/Supplier.o .obj/Supplier.so .shobj/Supplier.o .shobj/Supplier.so: Supplier.cpp Supplier.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventCommS.h \
  $(ACE_ROOT)/ace/pre.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventCommC.h \
  $(TAO_ROOT)/tao/ORB.h \
  $(TAO_ROOT)/tao/Exception.h \
  $(TAO_ROOT)/tao/orbconf.h \
  $(ACE_ROOT)/ace/Basic_Types.h \
  $(ACE_ROOT)/ace/post.h \
  $(ACE_ROOT)/ace/ace_wchar.h \
  $(ACE_ROOT)/ace/ace_wchar.inl \
  $(ACE_ROOT)/ace/OS_main.h \
  $(ACE_ROOT)/ace/os_include/os_limits.h \
  $(ACE_ROOT)/ace/os_include/os_unistd.h \
  $(ACE_ROOT)/ace/os_include/sys/os_types.h \
  $(ACE_ROOT)/ace/os_include/os_stddef.h \
  $(ACE_ROOT)/ace/os_include/os_inttypes.h \
  $(ACE_ROOT)/ace/os_include/os_stdint.h \
  $(ACE_ROOT)/ace/os_include/os_stdio.h \
  $(ACE_ROOT)/ace/os_include/os_stdarg.h \
  $(ACE_ROOT)/ace/os_include/os_float.h \
  $(ACE_ROOT)/ace/os_include/os_stdlib.h \
  $(ACE_ROOT)/ace/os_include/sys/os_wait.h \
  $(ACE_ROOT)/ace/os_include/os_signal.h \
  $(ACE_ROOT)/ace/os_include/os_time.h \
  $(ACE_ROOT)/ace/os_include/os_ucontext.h \
  $(ACE_ROOT)/ace/os_include/sys/os_resource.h \
  $(ACE_ROOT)/ace/os_include/sys/os_time.h \
  $(ACE_ROOT)/ace/ACE_export.h \
  $(ACE_ROOT)/ace/Global_Macros.h \
  $(ACE_ROOT)/ace/OS_Errno.h \
  $(ACE_ROOT)/ace/os_include/os_errno.h \
  $(ACE_ROOT)/ace/Synch_Traits.h \
  $(ACE_ROOT)/ace/Lock.h \
  $(ACE_ROOT)/ace/SStringfwd.h \
  $(ACE_ROOT)/ace/iosfwd.h \
  $(ACE_ROOT)/ace/CORBA_macros.h \
  $(ACE_ROOT)/ace/Exception_Macros.h \
  $(TAO_ROOT)/tao/TAO_Export.h \
  $(TAO_ROOT)/tao/Basic_Types.h \
  $(ACE_ROOT)/ace/CDR_Base.h \
  $(ACE_ROOT)/ace/Default_Constants.h \
  $(TAO_ROOT)/tao/objectid.h \
  $(TAO_ROOT)/tao/PolicyC.h \
  $(TAO_ROOT)/tao/CurrentC.h \
  $(TAO_ROOT)/tao/Object.h \
  $(TAO_ROOT)/tao/IOP_IORC.h \
  $(TAO_ROOT)/tao/OctetSeqC.h \
  $(TAO_ROOT)/tao/Sequence.h \
  $(TAO_ROOT)/tao/Managed_Types.h \
  $(TAO_ROOT)/tao/CORBA_String.h \
  $(TAO_ROOT)/tao/default_environment.h \
  $(TAO_ROOT)/tao/Sequence_T.h \
  $(TAO_ROOT)/tao/Objref_VarOut_T.h \
  $(TAO_ROOT)/tao/varbase.h \
  $(TAO_ROOT)/tao/Objref_VarOut_T.cpp \
  $(TAO_ROOT)/tao/Environment.h \
  $(TAO_ROOT)/tao/CORBA_methods.h \
  $(TAO_ROOT)/tao/Pseudo_VarOut_T.h \
  $(TAO_ROOT)/tao/Pseudo_VarOut_T.cpp \
  $(TAO_ROOT)/tao/Pseudo_VarOut_T.inl \
  $(TAO_ROOT)/tao/Objref_VarOut_T.inl \
  $(TAO_ROOT)/tao/Sequence_T.cpp \
  $(TAO_ROOT)/tao/Array_VarOut_T.h \
  $(TAO_ROOT)/tao/Array_VarOut_T.cpp \
  $(TAO_ROOT)/tao/Array_VarOut_T.inl \
  $(ACE_ROOT)/ace/OS_Memory.h \
  $(ACE_ROOT)/ace/OS_NS_stdlib.h \
  $(TAO_ROOT)/tao/Sequence_T.i \
  $(TAO_ROOT)/tao/Seq_Var_T.h \
  $(TAO_ROOT)/tao/Seq_Var_T.cpp \
  $(TAO_ROOT)/tao/Seq_Var_T.inl \
  $(TAO_ROOT)/tao/Seq_Out_T.h \
  $(TAO_ROOT)/tao/Seq_Out_T.cpp \
  $(TAO_ROOT)/tao/Seq_Out_T.inl \
  $(TAO_ROOT)/tao/CDR.h \
  $(ACE_ROOT)/ace/CDR_Stream.h \
  $(ACE_ROOT)/ace/Message_Block.h \
  $(ACE_ROOT)/ace/Time_Value.h \
  $(ACE_ROOT)/ace/Message_Block_T.h \
  $(ACE_ROOT)/ace/Message_Block_T.cpp \
  $(ACE_ROOT)/ace/Malloc_Base.h \
  $(ACE_ROOT)/ace/os_include/sys/os_mman.h \
  $(ACE_ROOT)/ace/Message_Block_T.i \
  $(TAO_ROOT)/tao/VarOut_T.h \
  $(TAO_ROOT)/tao/VarOut_T.cpp \
  $(TAO_ROOT)/tao/VarOut_T.inl \
  $(TAO_ROOT)/tao/Policy_ForwardC.h \
  $(TAO_ROOT)/tao/Object_Argument_T.h \
  $(TAO_ROOT)/tao/Argument.h \
  $(TAO_ROOT)/tao/Object_Argument_T.cpp \
  $(TAO_ROOT)/tao/Dynamic_ParameterC.h \
  $(TAO_ROOT)/tao/ParameterMode.h \
  $(TAO_ROOT)/tao/Any.h \
  $(TAO_ROOT)/tao/Arg_Traits_T.h \
  $(TAO_ROOT)/tao/Object_Argument_T.inl \
  $(TAO_ROOT)/tao/OBV_Constants.h \
  $(ACE_ROOT)/ace/Thread_Mutex.h \
  $(ACE_ROOT)/ace/OS_NS_Thread.h \
  $(ACE_ROOT)/ace/os_include/os_pthread.h \
  $(ACE_ROOT)/ace/Base_Thread_Adapter.h \
  $(ACE_ROOT)/ace/OS_Log_Msg_Attributes.h \
  $(ACE_ROOT)/ace/os_include/sys/os_sem.h \
  $(ACE_ROOT)/ace/os_include/sys/os_ipc.h \
  $(ACE_ROOT)/ace/os_include/os_semaphore.h \
  $(ACE_ROOT)/ace/OS_NS_signal.h \
  $(ACE_ROOT)/ace/Guard_T.h \
  $(ACE_ROOT)/ace/Guard_T.cpp \
  $(ACE_ROOT)/ace/Guard_T.inl \
  $(ACE_ROOT)/ace/RW_Thread_Mutex.h \
  $(ACE_ROOT)/ace/RW_Mutex.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEvent/event_export.h \
  $(TAO_ROOT)/tao/Collocation_Proxy_Broker.h \
  $(TAO_ROOT)/tao/Collocation_Strategy.h \
  $(TAO_ROOT)/tao/PortableServer/PortableServer.h \
  $(TAO_ROOT)/tao/PortableServer/portableserver_export.h \
  $(TAO_ROOT)/tao/PortableServer/PortableServerC.h \
  $(TAO_ROOT)/tao/PortableServer/Servant_Base.h \
  $(TAO_ROOT)/tao/Abstract_Servant_Base.h \
  $(ACE_ROOT)/ace/Atomic_Op.h \
  $(ACE_ROOT)/ace/Atomic_Op_T.h \
  $(ACE_ROOT)/ace/Atomic_Op_T.cpp \
  $(ACE_ROOT)/ace/Log_Msg.h \
  $(ACE_ROOT)/ace/Log_Priority.h \
  $(ACE_ROOT)/ace/Atomic_Op_T.i \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventCommS_T.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventCommS_T.cpp \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventCommS_T.i \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventChannelAdminS.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventChannelAdminC.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventChannelAdminS_T.h \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventChannelAdminS_T.cpp \
  $(TAO_ROOT)/orbsvcs/orbsvcs/CosEventChannelAdminS_T.i \
  $(ACE_ROOT)/ace/OS_NS_unistd.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
