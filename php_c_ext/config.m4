dnl $Id$
dnl config.m4 for extension nxytest

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(nxytest, for nxytest support,
dnl Make sure that the comment is aligned:
dnl [  --with-nxytest             Include nxytest support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(nxytest, whether to enable nxytest support,
Make sure that the comment is aligned:
[  --enable-nxytest           Enable nxytest support])

if test "$PHP_NXYTEST" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-nxytest -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/nxytest.h"  # you most likely want to change this
  dnl if test -r $PHP_NXYTEST/$SEARCH_FOR; then # path given as parameter
  dnl   NXYTEST_DIR=$PHP_NXYTEST
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for nxytest files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       NXYTEST_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$NXYTEST_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the nxytest distribution])
  dnl fi

  dnl # --with-nxytest -> add include path
  dnl PHP_ADD_INCLUDE($NXYTEST_DIR/include)

  dnl # --with-nxytest -> check for lib and symbol presence
  dnl LIBNAME=nxytest # you may want to change this
  dnl LIBSYMBOL=nxytest # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $NXYTEST_DIR/$PHP_LIBDIR, NXYTEST_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_NXYTESTLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong nxytest lib version or lib not found])
  dnl ],[
  dnl   -L$NXYTEST_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(NXYTEST_SHARED_LIBADD)

  PHP_NEW_EXTENSION(nxytest, nxytest.c trie.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
