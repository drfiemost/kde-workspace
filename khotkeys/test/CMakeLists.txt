MACRO(KHOTKEYS_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} NOGUI ${_testname}.cpp)
               target_link_libraries(${_testname} khotkeysprivate ${KDE4_KDECORE_LIBS} Qt4::QtTest)
       ENDFOREACH(_testname)
ENDMACRO(KHOTKEYS_UNIT_TESTS)

KHOTKEYS_UNIT_TESTS(
    kdedmodule_daemon_test )

