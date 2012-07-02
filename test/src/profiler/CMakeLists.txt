add_definitions(-DGTEST_LINKED_AS_SHARED_LIBRARY -DENABLE_PROFILER)
if(LINK_STATICALLY)
	add_definitions(-DPROFILER_LINK_STATICALLY)
endif()

set (TARGET TestProfiler)

file(GLOB SOURCES "*.cpp")

find_package(gtest REQUIRED)

SET (DEPENDENCIES ${DEPENDENCIES} Profiler gtest)

add_executable (${TARGET} ${HEADERS} ${SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

install (TARGETS ${TARGET} RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

SetVSTargetProperties(${TARGET})