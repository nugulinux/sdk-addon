SET(UNIT_TESTS
    test_alarm)

FOREACH(test ${UNIT_TESTS})
	ADD_EXECUTABLE(${test}
        ${test}.cc
        $<TARGET_OBJECTS:objaddon>
    )
	TARGET_COMPILE_DEFINITIONS(${test} PRIVATE
		-DRUNPATH="${CMAKE_CURRENT_BINARY_DIR}")
    TARGET_INCLUDE_DIRECTORIES(${test} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src)
	TARGET_LINK_LIBRARIES(${test} ${pkgs_LDFLAGS} -lstdc++)
	ADD_TEST(${test} ${test})
ENDFOREACH(test)
