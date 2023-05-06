/****************************************************************************
 *      This is part of the libYafaRay-Xml package
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2.1 of the License, or (at your option) any later version.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library; if not, write to the Free Software
 *      Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "public_api/yafaray_xml_c_api.h"
#include "import/import_xml.h"
#include "common/version_build_info.h"
#include <cstring>

yafaray_Container *yafaray_xml_ParseFile(yafaray_Logger *yafaray_logger, const char *xml_file_path, const char *input_color_space, float input_gamma)
{
	auto [result, scene, surface_integrator, film]{yafaray_xml::XmlParser::parseXmlFile(yafaray_logger, xml_file_path, input_color_space, input_gamma)};
	auto container{yafaray_createContainer()};
	yafaray_addSceneToContainer(container, scene);
	yafaray_addSurfaceIntegratorToContainer(container, surface_integrator);
	yafaray_addFilmToContainer(container, film);
	return container;
}

yafaray_Container *yafaray_xml_ParseMemory(yafaray_Logger *yafaray_logger, const char *xml_buffer, int xml_buffer_size, const char *input_color_space, float input_gamma)
{
	auto [result, scene, surface_integrator, film]{yafaray_xml::XmlParser::parseXmlMemory(yafaray_logger, xml_buffer, xml_buffer_size, input_color_space, input_gamma)};
	auto container{yafaray_createContainer()};
	yafaray_addSceneToContainer(container, scene);
	yafaray_addSurfaceIntegratorToContainer(container, surface_integrator);
	yafaray_addFilmToContainer(container, film);
	return container;
}

char *createCString(const std::string &std_string)
{
	const size_t string_size = std_string.size();
	char *c_string = new char[string_size + 1];
	std::strcpy(c_string, std_string.c_str());
	return c_string;
}

char *yafaray_xml_getVersionString()
{
	return createCString(yafaray_xml::build_info::getVersionString());
}

int yafaray_xml_getVersionMajor() { return yafaray_xml::build_info::getVersionMajor(); }
int yafaray_xml_getVersionMinor() { return yafaray_xml::build_info::getVersionMinor(); }
int yafaray_xml_getVersionPatch() { return yafaray_xml::build_info::getVersionPatch(); }

void yafaray_xml_destroyCharString(char *string)
{
	delete[] string;
}