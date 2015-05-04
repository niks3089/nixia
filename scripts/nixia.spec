Summary: Nixia traffic generator
Name: nixia
Version: 0.5
Packager: Nikhil <niks3089@gmail.com>
Group: Network traffic generator 
License: GPL

%define build_timestamp %(date +"%Y%b%d%H%m%%S ")

Release:  %{build_timestamp}

%description
Brief description of software package.

%prep

%build

%install
mkdir -p %_topdir/BUILDROOT/nixia-%{version}-%{release}.%_target_cpu/usr/local/bin
mkdir -p %_topdir/BUILDROOT/nixia-%{version}-%{release}.%_target_cpu/etc/nixia
mkdir -p %_topdir/BUILDROOT/nixia-%{version}-%{release}.%_target_cpu/usr/local/share/man/man1/

cp -rf %_mypath/build/bin/nixia %_topdir/BUILDROOT/nixia-%{version}-%{release}.%_target_cpu/usr/local/bin/
cp -rf %_mypath/src/config/log/log.conf %_topdir/BUILDROOT/nixia-%{version}-%{release}.%_target_cpu/etc/nixia/
cp -rf %_mypath/docs/nixia.1 %_topdir/BUILDROOT/nixia-%{version}-%{release}.%_target_cpu/usr/local/share/man/man1/

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

%attr(755, root, root) /usr/local/bin/nixia
%attr(777, root, root) /etc/nixia/log.conf
%attr(644, root, root) /usr/local/share/man/man1/nixia.1

%doc

%changelog
