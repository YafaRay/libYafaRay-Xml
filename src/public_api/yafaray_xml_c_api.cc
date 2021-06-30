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

bool yafaray_xml_ParseFile(yafaray_Interface_t *yafaray_interface, const char *xml_file_path)
{
	return yafaray_xml::XmlParser::parseXmlFile(yafaray_interface, xml_file_path);
}

bool yafaray_xml_ParseMemory(yafaray_Interface_t *yafaray_interface, const char *xml_buffer, unsigned int xml_buffer_size)
{
	return yafaray_xml::XmlParser::parseXmlMemory(yafaray_interface, xml_buffer, xml_buffer_size);
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
	return createCString(yafaray_xml::buildinfo::getVersionString());
}

int yafaray_xml_getVersionMajor() { return yafaray_xml::buildinfo::getVersionMajor(); }
int yafaray_xml_getVersionMinor() { return yafaray_xml::buildinfo::getVersionMinor(); }
int yafaray_xml_getVersionPatch() { return yafaray_xml::buildinfo::getVersionPatch(); }

void yafaray_xml_deallocateCharPointer(char *string_pointer_to_deallocate)
{
	delete[] string_pointer_to_deallocate;
}