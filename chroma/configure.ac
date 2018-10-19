dnl Process this file with autoconf to produce a configure script.
AC_INIT(main.c)

AC_ARG_ENABLE(sdl,
	[  --enable-sdl            build SDL chroma],
        [with_sdl=$enableval],
        [with_sdl=yes],
)

AC_ARG_ENABLE(curses,
	[  --enable-curses         build curses chroma],
        [with_curses=$enableval],
        [with_curses=yes],
)

dnl Check for programs
AC_PROG_CC
AC_PROG_INSTALL

dnl Check for SDL
if test "$with_sdl" = "yes"; then
    AC_PATH_PROG(SDLCONFIG, sdl-config, no)
    if test "$SDLCONFIG" = "no"; then
        AC_MSG_ERROR([*** Unable to find sdl-config])
    else
        CFLAGS="$CFLAGS "`sdl-config --cflags`
        LDFLAGS="$LDFLAGS "`sdl-config --libs`
    fi
fi

dnl Check for SDL_image library
if test "$with_sdl" = "yes"; then
    AC_CHECK_LIB(SDL_image, IMG_LoadPNG_RW)
    if test "$ac_cv_lib_SDL_image_IMG_LoadPNG_RW" != "yes"; then
        AC_MSG_ERROR([*** Unable to find SDL_image library with PNG support])
    fi
fi

dnl Check for the FreeType 2 library
if test "$with_sdl" = "yes"; then
    AC_ARG_WITH(freetype-prefix,[  --with-freetype-prefix=PFX   Prefix where FREETYPE is installed (optional)], freetype_prefix="$withval", freetype_prefix="")
    AC_ARG_WITH(freetype-exec-prefix,[  --with-freetype-exec-prefix=PFX Exec prefix where FREETYPE is installed (optional)], freetype_exec_prefix="$withval", freetype_exec_prefix="")
    if test x$freetype_exec_prefix != x ; then
        freetype_args="$freetype_args --exec-prefix=$freetype_exec_prefix"
        if test x${FREETYPE_CONFIG+set} != xset ; then
            FREETYPE_CONFIG=$freetype_exec_prefix/bin/freetype-config
        fi
    fi
    if test x$freetype_prefix != x ; then
        freetype_args="$freetype_args --prefix=$freetype_prefix"
        if test x${FREETYPE_CONFIG+set} != xset ; then
        FREETYPE_CONFIG=$freetype_prefix/bin/freetype-config
        fi
    fi
    AC_PATH_PROG(FREETYPE_CONFIG, freetype-config, no)
    no_freetype=""
    if test "$FREETYPE_CONFIG" = "no" ; then
        AC_MSG_ERROR([*** Unable to find FreeType2 library (http://www.freetype.org/)])
    else
        CFLAGS="$CFLAGS `$FREETYPE_CONFIG $freetypeconf_args --cflags`"
        LIBS="$LIBS `$FREETYPE_CONFIG $freetypeconf_args --libs`"
    fi
fi

dnl Check for curses library
if test "$with_curses" = "yes"; then
    CURSES="ncurses"
    AC_CHECK_LIB(ncurses, initscr)
    if test "$ac_cv_lib_ncurses_initscr" != "yes"; then
        CURSES="curses"
        AC_CHECK_LIB(curses, initscr)
        if test "$ac_cv_lib_curses_initscr" != "yes"; then
            CURSES=""
        fi
    fi
    if test "$CURSES" = ""; then
    AC_MSG_ERROR([*** Unable to find curses / ncurses library])
    fi
fi

dnl Check for gettext library
AC_SEARCH_LIBS(gettext, intl)

dnl Determine targets to build
TARGETS=""
TARGETSCLEAN=""
TARGETSINSTALL=""
if test "$with_sdl" = "yes"; then
TARGETS="$TARGETS chroma-sdl"
TARGETSCLEAN="$TARGETSCLEAN clean-sdl";
TARGETSINSTALL="$TARGETSINSTALL install-sdl";
fi
if test "$with_curses" = "yes"; then
TARGETS="$TARGETS chroma-curses"
TARGETSCLEAN="$TARGETSCLEAN clean-curses";
TARGETSINSTALL="$TARGETSINSTALL install-curses";
fi
if test "$TARGETS" = ""; then
AC_MSG_ERROR([*** Nothing to build - specify either --enable-sdl and/or --enable-curses])
fi

AC_SUBST(CURSES)
AC_SUBST(TARGETS)
AC_SUBST(TARGETSCLEAN)
AC_SUBST(TARGETSINSTALL)
AC_OUTPUT(Makefile)

