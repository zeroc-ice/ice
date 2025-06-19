# Copyright (c) ZeroC.

Name: ice-repo-3.8
Version: 1.0.0
Summary: Yum repo configuration for ZeroC Ice 3.8
Release: 1%{?dist}
License: GPLv2 with exceptions
Vendor: ZeroC, Inc.
URL: https://zeroc.com/
Source0: zeroc-ice-3.8.repo
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
* Thu Jun 19 2025 Jose Gutierrez de la Concha <jose@zeroc.com> 1.0.0-1
- Add repository for ZeroC Ice 3.8 builds
