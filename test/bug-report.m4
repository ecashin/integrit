dnl
#! /bin/sh
# integrit - file integrity verification system
# Copyright (C) 2006 Ed L. Cashin
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# 
define(`myself', `bug-report')dnl
define(`abort', `{ echo "myself: FATAL failure: $1" 1>&2; exit 1; }')dnl
define(`do_report',dnl
`{ echo "myself: ACTION FAILED: $1" 1>&2;dnl
 do_bundle($rptdir); rm -rf $rptdir; exit; }')dnl
define(`try', `$2 || abort($1)')dnl
define(`do_test', `# $1
echo $me: $1
$2 || do_report($1)')dnl
define(`do_bundle',dnl
try(`bundle myself',dnl
`(set -e; rm -f myself.tar myself.tar.gz;dnl
 tar cf myself.tar $rptdir; gzip myself.tar)'))dnl

# NOTE: the file, "bug-report", is generated from the file, "bug-report.m4"

me=myself; export me
rptdir=$me-$$; export rptdir
make=${MAKE:-make}; export make
echo $me: using temporary working directory: $rptdir
echo $me: $make clean
$make clean 2> /dev/null
try(`create directory for bug report', `mkdir $rptdir')
changequote([, ])dnl
do_test([configuring integrit],dnl
[(./configure > $rptdir/$me-configure.out; success=$?;dnl
 cp config.log config.h Makefile $rptdir;dnl
 exit $success)])
do_test([making integrit], [$make > $rptdir/$me-$make.out])
echo $me: creating myself.tar.gz
do_bundle

rm -rf $rptdir
