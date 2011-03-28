AC_INIT(src/midiio.cpp)
AM_INIT_AUTOMAKE(midiio, 0.1)
AC_PROG_CXX
AC_LANG_C
dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC

dnl Check for headers
AC_CHECK_HEADERS(unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T


dnl Detect OS we're building on
compile_target=$host
echo "try 1"
echo "Host Value: \'${compile_target}'"

case "${host}" in
	*-mingw32*)
	compile_target=win
	CPPFLAGS="$CPPFLAGS -D__WINDOWS_MM__"
	LIBS="$LIBS -lwinmm"
	;;
	*-darwin*)
	dnl we should add checks here to make sure osx dev lib's are installed
	CPPFLAGS="$CPPFLAGS -D__MACOSX_CORE__"
	LIBS="$LIBS -framework CoreMidi -framework CoreAudio -framework CoreFoundation"
	;;
	*)
	compile_target=linux
	dnl Check for ALSA
	AC_CHECK_LIB(asound, snd_seq_event_output_direct,alsalib=yes,alsalib=no)
	AC_CHECK_HEADERS(alsa/asoundlib.h,alsaheader=yes,alsaheader=no)

	if test "$alsalib" = "yes"; then
		if test "$alsaheader" = "yes"; then
			LIBS="$LIBS -lasound"
		else
			AC_MSG_ERROR([** Coulnd't find ALSA header file sys/asoundlib.h **])
		fi
	else
		AC_MSG_ERROR([** Couldn't find ALSA library libasound. **])
	fi
	CPPFLAGS="$CPPFLAGS -D__LINUX_ALSASEQ__"
	;;
esac

AC_HEADER_STDC
AM_CONFIG_HEADER(config.h)
AC_OUTPUT(Makefile src/Makefile)
