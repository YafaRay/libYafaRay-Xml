add_executable(yafaray_xml_loader loader_xml.cc)
target_link_libraries(yafaray_xml_loader LibYafaRay::libyafaray4 libyafaray4_xml)
target_include_directories(yafaray_xml_loader PRIVATE ${PROJECT_BINARY_DIR}/include)
set_target_properties(yafaray_xml_loader PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

install(TARGETS yafaray_xml_loader
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		)
