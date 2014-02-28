Summary: integrit is a file verification system.
Name: integrit 
# major/minor version level
%define majorminorv 1.05
# patch level of this major/minor version
%define patchlevel 03
Version: %{majorminorv}.%{patchlevel}
# rpm release number
Release: 2
Copyright: GPL v2
Group: Application 
Source0: http://download.sourceforge.net/integrit/integrit-%{majorminorv}.%{patchlevel}.tar.gz 
URL: http://integrit.sourceforge.net 
Vendor: Ed L Cashin <ecashin@users.sourceforge.net>  
Packager: Gyepi Sam <gyepi@peanutpress.com> 
Buildroot: /var/tmp/%{name}-%{majorminorv}

%description
integrit is a simple yet secure alternative to products like tripwire.
It has a small memory footprint, uses up to date cryptographic
algorithms, and has features that make sense (like including the MD5
checksum of newly generated databases in the report).

%prep

%setup -n %{name}-%{majorminorv}

%build
./configure --prefix=${RPM_BUILD_ROOT}%{_prefix}
make 
make aux

%install
make install

%clean
rm -rf $RPM_BUILD_ROOT

%post
echo
echo 'It is recommended that the binary be copied to a secure location and'
echo "  re-copied to %{_prefix}/sbin at runtime or run directly"
echo "  from the secure medium."

%files
%defattr(-,root,root)
%doc README todo.txt examples Changes INSTALL LICENSE 
#integrit.spec
%{_prefix}/man/man1/i-ls.1
%{_prefix}/man/man1/i-viewdb.1
%{_prefix}/man/man1/integrit.1
%{_prefix}/sbin/integrit
%{_prefix}/sbin/i-viewdb
%{_prefix}/bin/i-ls

%changelog

* Tue Feb 13 2001 Gyepi Sam <gyepi@peanutpress.com>
- added post install message

* Mon Feb 12 2001 Ed L. Cashin <ecashin@users.sourceforge.net>
- removed openssl requirement, since openssl is not needed to run integrit
- rpm version number is integrit version number, distinct from package release
- sync source URL with above change

* Sun Feb 11 2001 Gyepi Sam <gyepi@peanutpress.com>
- added openssl requirement

* Sat Feb 10 2001 Gyepi Sam <gyepi@peanutpress.com>
- initial rpm spec

# Local variables:
#  compile-command: "rpm -ba integrit.spec"
# End:
