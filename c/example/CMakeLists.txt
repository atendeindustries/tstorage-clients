add_executable(tstorage-example
	tstorage-example.c
)

target_include_directories(tstorage-example PRIVATE ../include)

target_link_libraries(tstorage-example tstorage-client)
