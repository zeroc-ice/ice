#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_server_impl     = Test
$(test)_serveramd_impl  = TestAMD

#
# $(call make-impl-with-config,$1=project,$2=component,$3=platform,$4=config,$5=comp[platform-config])
#
define make-impl-with-config

ifneq ($$($2_impl),)
$5_sources      += $$($5_objdir)/$$($2_impl)I.cpp
$5_objects      += $$(addprefix $$($5_objdir)/,$$(call source-to-object,$$($5_objdir)/$$($2_impl)I.cpp))
$5_cppflags     += -I$$($5_objdir) $(nounusedparameter-cppflags)

$$($5_objects): $$($5_objdir)/$$($2_impl)I.cpp

ifeq ($$(filter $$($5_objdir)/$$($2_impl)I.cpp,$$($1_impls)),)

$$($5_objdir)/$$($2_impl)I.cpp: $1/$$($2_impl).ice $(slice2cpp_path)
	$(E) "Compiling $$< implementation"
	$(Q)$(MKDIR) -p $$(*D)
	$(Q)$(RM) $$(*).h $$(*).cpp
	$(Q)$(slice2cpp_path) $$($2_sliceflags) $$(if $$(filter cpp11-%,$4),--impl-c++11,--impl-c++98) --output-dir $$(*D) $$<

# Prevents warning if multiple configurations refer to $$($5_objdir)/$$($2_impl)I.cpp
$1_impls += $$($5_objdir)/$$($2_impl)I.cpp

endif

endif

endef

$(test)_component_with_config_extensions = make-impl-with-config

tests += $(test)
