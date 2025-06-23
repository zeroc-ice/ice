# Copyright (c) ZeroC.

Name: ice-repo
Version: 3.8
Summary: Yum repo configuration for ZeroC Ice
Release: 1%{?dist}
License: GPLv2 with exceptions
Vendor: ZeroC, Inc.
URL: https://zeroc.com/
Source0: zeroc-ice3.8.repo
Group: System Environment/Base
BuildArch: noarch

%description
This package installs the yum repo file for the 3.8 release of the ZeroC Ice RPC framework.

%prep
%setup -q -T -c

%install
install -dm 755 %{buildroot}%{_sysconfdir}/yum.repos.d
install -pm 644 %{SOURCE0} %{buildroot}%{_sysconfdir}/yum.repos.d/zeroc-ice-3.8.repo

%files
%defattr(-,root,root,-)
%config(noreplace) %{_sysconfdir}/yum.repos.d/zeroc-ice3.8.repo

%changelog
* Tue Jun 17 2025 Jose Gutierrez de la Concha <jose@zeroc.com> 3.8-1
- 3.8 release

* Thu Feb 28 2019 Bernard Normier <bernard@zeroc.com> - 3.7-1
- Initial package
