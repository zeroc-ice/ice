# Copyright (c) ZeroC.

Name: ice-nightly-repo
Version: 3.8
Summary: Yum repo configuration for ZeroC Ice Nightly builds
Release: 1%{?dist}
License: GPLv2 with exceptions
Vendor: ZeroC, Inc.
URL: https://zeroc.com/
Source0: zeroc-ice-nightly.repo
Group: System Environment/Base
BuildArch: noarch

%description
This package installs the yum repo file for the Nightly builds of the ZeroC Ice RPC framework.

%prep
%setup -q -T -c

%install
install -dm 755 %{buildroot}%{_sysconfdir}/yum.repos.d
install -pm 644 %{SOURCE0} %{buildroot}%{_sysconfdir}/yum.repos.d/zeroc-ice-nightly.repo

%files
%defattr(-,root,root,-)
%config(noreplace) %{_sysconfdir}/yum.repos.d/zeroc-ice-nightly.repo

%changelog
* Thu Jun 19 2025 Jose Gutierrez de la Concha <jose@zeroc.com> 3.8-1
- Add repository for ZeroC Ice 3.8 nightly builds
