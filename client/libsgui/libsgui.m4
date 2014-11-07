# stolen from Sam Lantinga 9/21/99
# which has stolen it from Manish Singh
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_SGUI([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for libsgui, and define SGUI_CFLAGS and SGUI_LIBS
dnl
AC_DEFUN([AM_PATH_SGUI],
[dnl 
dnl Get the cflags and libraries from the sdl-config script
dnl
AC_ARG_WITH(sgui-prefix,[  --with-sgui-prefix=PFX   Prefix where libsgui is installed (optional)],
            sgui_prefix="$withval", sgui_prefix="")
AC_ARG_WITH(sgui-exec-prefix,[  --with-sgui-exec-prefix=PFX Exec prefix where libsgui is installed (optional)],
            sgui_exec_prefix="$withval", sgui_exec_prefix="")
AC_ARG_ENABLE(sguitest, [  --disable-sguitest       Do not try to compile and run a test sgui program],
		    , enable_sguitest=yes)

  if test x$sgui_exec_prefix != x ; then
     sgui_args="$sgui_args --exec-prefix=$sgui_exec_prefix"
     if test x${SGUI_CONFIG+set} != xset ; then
        SGUI_CONFIG=$sgui_exec_prefix/bin/libsgui-config
     fi
  fi
  if test x$sgui_prefix != x ; then
     sgui_args="$sgui_args --prefix=$sgui_prefix"
     if test x${SGUI_CONFIG+set} != xset ; then
        SGUI_CONFIG=$sgui_prefix/bin/libsgui-config
     fi
  fi

  AC_REQUIRE([AC_CANONICAL_TARGET])
  PATH="$prefix/bin:$prefix/usr/bin:$PATH"
  AC_PATH_PROG(SGUI_CONFIG, libsgui-config, no, [$PATH])
  min_sgui_version=ifelse([$1], ,0.11.0,$1)
  AC_MSG_CHECKING(for libsgui - version >= $min_sgui_version)
  no_sgui=""
  if test "$SGUI_CONFIG" = "no" ; then
    no_sgui=yes
  else
    SGUI_CFLAGS=`$SGUI_CONFIG $sguiconf_args --cflags`
    SGUI_LIBS=`$SGUI_CONFIG $sguiconf_args --libs`

    sgui_major_version=`$SGUI_CONFIG $sgui_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    sgui_minor_version=`$SGUI_CONFIG $sgui_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    sgui_micro_version=`$SGUI_CONFIG $SGUI_CONFIG_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_sguitest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $SGUI_CFLAGS"
      LIBS="$LIBS $SGUI_LIBS"
dnl
dnl Now check if the installed sgui is sufficiently new. (Also sanity
dnl checks the results of libsgui-config to some extent
dnl
      rm -f conf.sguitest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libsgui/io.h>

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main (int argc, char *argv[])
{
  int major, minor, micro;
  char *tmp_version;

  /* This hangs on some systems (?)
  system ("touch conf.sguitest");
  */
  { FILE *fp = fopen("conf.sguitest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_sgui_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_sgui_version");
     exit(1);
   }

   if (($sgui_major_version > major) ||
      (($sgui_major_version == major) && ($sgui_minor_version > minor)) ||
      (($sgui_major_version == major) && ($sgui_minor_version == minor) && ($sgui_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'libsgui-config --version' returned %d.%d.%d, but the minimum version\n", $sgui_major_version, $sgui_minor_version, $sgui_micro_version);
      printf("*** of sgui required is %d.%d.%d. If libsgui-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If libsgui-config was wrong, set the environment variable SGUI_CONFIG\n");
      printf("*** to point to the correct copy of libsgui-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_sgui=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_sgui" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$SGUI_CONFIG" = "no" ; then
       echo "*** The libsgui-config script installed by libsgui could not be found"
       echo "*** If libsgui was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the SGUI_CONFIG environment variable to the"
       echo "*** full path to libsgui-config."
     else
       if test -f conf.sguitest ; then
        :
       else
          echo "*** Could not run sgui test program, checking why..."
          CFLAGS="$CFLAGS $SGUI_CFLAGS"
          LIBS="$LIBS $SGUI_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include "sgui.h"

int main(int argc, char *argv[])
{ return 0; }
#undef  main
#define main K_and_R_C_main
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding sgui or finding the wrong"
          echo "*** version of sgui. If it is not finding sgui, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means sgui was incorrectly installed"
          echo "*** or that you have moved sgui since it was installed. In the latter case, you"
          echo "*** may want to edit the libsgui-config script: $SGUI_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     SGUI_CFLAGS=""
     SGUI_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(SGUI_CFLAGS)
  AC_SUBST(SGUI_LIBS)
  rm -f conf.sguitest
])
