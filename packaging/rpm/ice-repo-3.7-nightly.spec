# Copyright (c) ZeroC.

Name: ice-repo-3.9-nightly
Version: 1.0.0
Summary: Yum repo configuration for ZeroC Ice 3.9 Nightly builds
Release: 1%{?dist}
License: GPLv2 with exceptions
Vendor: ZeroC, Inc.
URL: https://zeroc.com/
Source0: zeroc-ice-3.9-nightly.repo
Group: System Environment/Base
BuildArch: noarch

%description
This package installs the yum repo file for the Nightly builds of the ZeroC Ice RPC framework.

%prep
%setup -q -T -c

%install
install -dm 755 %{buildroot}%{_sysconfdir}/yum.repos.d
install -pm 644 %{SOURCE0} %{buildroot}%{_sysconfdir}/yum.repos.d/zeroc-ice-3.9-nightly.repo

%files
%defattr(-,root,root,-)
%config(noreplace) %{_sysconfdir}/yum.repos.d/zeroc-ice-3.9-nightly.repo

%changelog
* Mon Dec 22 2025 Jose Gutierrez de la Concha <jose@zeroc.com> 1.0.0-1
- Add repository for ZeroC Ice 3.9 Nightly builds
