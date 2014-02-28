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
# NOTE: the file, "test", is generated from the file, "test.m4"

dnl more sane quotes
changequote([, ])

dnl local macros
define([myself], [test])
define([abort], [{ echo "myself: TEST FAILED: $1" 1>&2; rm -rf $testdir; exit 1; }])
define([try], [$2 || abort($1)])
define([integritcheck], [./integrit -C $testdir/test.conf -c -u > $testdir/test.out; status=$?])
define([integritupdate], [./integrit -C $testdir/test.conf -u; status=$?])

scriptdir="`dirname $0`"
set -x

# initial tests
try([running integrit binary], [./integrit -V])
testdir=test-$$
try([creating test area], [$scriptdir/make-filetree $testdir])
try([creating config file], [$scriptdir/make-config $testdir])
try([integrit update], integritupdate)
try([check normal exit status], [test $status = 0])
try([checking for current database], [test -f $testdir/curr.cdb])
try([installing current db], [mv $testdir/curr.cdb $testdir/known.cdb])
try([integrit check and update], integritcheck)
try([check changed exit status], [test $status = 1])
try([make sure update created new database], [test -f $testdir/curr.cdb])
try([check for recognition that curr.cdb has changed],dnl
 [grep "changed: .*curr\.cdb" $testdir/test.out])
try([check for recognition that known.cdb is new],dnl
 [grep "new: .*known\.cdb" $testdir/test.out])

# file change
modfile=$testdir/data/two/three
modfile_esc="$testdir\/data\/two\/three"
try([modify a data file], [echo new contents > $modfile])
try([install the database], [mv $testdir/curr.cdb $testdir/known.cdb])
try([integrit check and update], integritcheck)
try([check changed exit status], [test $status = 1])
try([make sure update created new database], [test -f $testdir/curr.cdb])
try([make sure output acknowledged curr.cdb has changed],dnl
 [grep "changed: .*curr\.cdb" $testdir/test.out])
try([make sure output acknowledged curr.cdb has changed],dnl
 [grep "changed: .*$modfile_esc .*s[(]" $testdir/test.out])

# file removal
modfile=$testdir/data/two/three
modfile_esc="$testdir\/data\/two\/three"
try([creating new file], [$scriptdir/make-config -i $testdir])
try([integrit update], integritupdate)
try([install the database], [mv $testdir/curr.cdb $testdir/known.cdb])
try([integrit check and update], integritcheck)
try([check unchanged exit status], [test $status = 0])
try([remove a file], [rm $modfile])
try([integrit check and update], integritcheck)
try([check changed exit status], [test $status = 1])
try([make sure output acknowledged $modfile has changed],dnl
 [grep "missing: .*$modfile_esc .*s[(]" $testdir/test.out])

# unreadable file
modfile=$testdir/data/two/three
try([making a file unreadable], [touch $modfile && chmod 000 $modfile])
try([integrit check and update], integritcheck)
try([check error exit status], [test $status = 2])

# cleanup
rm -rf $testdir
set +x
printf "\neverything's fine.\n"
