file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*_tests.cpp")

foreach(TEST_SOURCE IN LISTS TEST_SOURCES)
	get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

	add_executable(${TEST_NAME}
		${TEST_SOURCE}
	)

	target_compile_options(${TEST_NAME} PRIVATE -Wall -Wextra -pedantic)
	target_compile_options(${TEST_NAME} PRIVATE -g)

	target_include_directories(${TEST_NAME}
		PRIVATE
			${PROJECT_SOURCE_DIR}/include
	)

	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
