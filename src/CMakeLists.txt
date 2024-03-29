add_library(libyafaray4_xml)
set_target_properties(libyafaray4_xml PROPERTIES PREFIX "" VERSION 4.0.0 SOVERSION 4)
set_target_properties(libyafaray4_xml PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
set_target_properties(libyafaray4_xml PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
target_include_directories(libyafaray4_xml PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
target_include_directories(libyafaray4_xml INTERFACE $<INSTALL_INTERFACE:include>)
target_link_libraries(libyafaray4_xml PRIVATE LibYafaRay::libyafaray4 LibXml2::LibXml2)

add_subdirectory(common)
add_subdirectory(import)
add_subdirectory(public_api)

# Custom definitions
target_compile_definitions(libyafaray4_xml
		PRIVATE
		"YAFARAY_XML_BUILD_TYPE=\"$<UPPER_CASE:$<CONFIG>>\""
		"YAFARAY_XML_BUILD_FLAGS=\"${CMAKE_CXX_FLAGS} $<$<CONFIG:Debug>:${CMAKE_CXX_FLAGS_DEBUG}>$<$<CONFIG:Release>:${CMAKE_CXX_FLAGS_RELEASE}>$<$<CONFIG:RelWithDebInfo>:${CMAKE_CXX_FLAGS_RELWITHDEBINFO}>$<$<CONFIG:MinSizeRel>:${CMAKE_CXX_FLAGS_MINSIZEREL}>\"")

# Custom linker options
if(CMAKE_SYSTEM_NAME MATCHES "Linux" AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
	target_link_options(libyafaray4_xml PRIVATE
			"LINKER:SHELL:--no-undefined"
			"LINKER:SHELL:--version-script=${PROJECT_SOURCE_DIR}/include/public_api/yafaray_xml_c_api_symbols.map" # Custom symbols export and versioning map
			"$<$<CONFIG:RELEASE>:LINKER:SHELL:-s>" # Strip in release builds
			)
endif()

# Copy C API include files to binary directory for other targets to use
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/include)
configure_file(${PROJECT_SOURCE_DIR}/include/public_api/yafaray_xml_c_api.h ${PROJECT_BINARY_DIR}/include/)

# Shared target used for generating export header for both the shared and static targets
include(GenerateExportHeader)
generate_export_header(libyafaray4_xml BASE_NAME yafaray_xml_c_api)
generate_export_header(libyafaray4_xml BASE_NAME yafaray_xml_c_api EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/yafaray_xml_c_api_export.h)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(${PROJECT_BINARY_DIR}/cmake/LibYafaRay_XmlConfigVersion.cmake
		COMPATIBILITY "AnyNewerVersion"
		)

install(TARGETS libyafaray4_xml
		EXPORT LibYafaRay_Xml
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		)

install(FILES
		${PROJECT_SOURCE_DIR}/include/public_api/yafaray_xml_c_api.h
		${PROJECT_BINARY_DIR}/include/yafaray_xml_c_api_export.h
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		)

install(EXPORT LibYafaRay_Xml
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LibYafaRay_Xml
		NAMESPACE LibYafaRay_Xml::
		FILE LibYafaRay_XmlConfig.cmake
		)

install(FILES
		${PROJECT_BINARY_DIR}/cmake/LibYafaRay_XmlConfigVersion.cmake
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LibYafaRay_Xml
		)