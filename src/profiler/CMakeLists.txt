set (TARGET Profiler)
add_definitions(-DPROFILER_BUILD -DENABLE_PROFILER)

file(GLOB HEADERS "*.h")

file(GLOB SOURCES "*.cpp")

if(LINK_STATICALLY)
	add_definitions(-DPROFILER_LINK_STATICALLY)
	add_library (${TARGET} STATIC ${HEADERS} ${SOURCES}) 
else()
	add_library (${TARGET} SHARED ${HEADERS} ${SOURCES}) 
	target_link_libraries (${TARGET} ${DEPENDENCIES})
endif()

set_target_properties (${TARGET} PROPERTIES
    SOVERSION ${PROJECT_SO_VERSION})

install (TARGETS ${TARGET} RUNTIME DESTINATION ${INSTALL_DIR}/bin LIBRARY DESTINATION ${INSTALL_DIR}/lib ARCHIVE DESTINATION ${INSTALL_DIR}/lib)
install (FILES ${HEADERS} DESTINATION ${INSTALL_DIR}/include/profiler)

SetVSTargetProperties(${TARGET})