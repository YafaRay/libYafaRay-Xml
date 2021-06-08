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

bool yafaray_xml_Parse(yafaray_Interface_t *yafaray_interface, const char *xml_file_path)
{
	return yafaray_xml::XmlParser::parseXml(yafaray_interface, xml_file_path);
}

void yafaray_xml_getVersionString(char *dest_string, unsigned int dest_string_size)
{
	if(!dest_string || dest_string_size == 0) return;
	const std::string version_string = yafaray_xml::buildinfo::getVersionString();
	const unsigned int copy_length = std::min(dest_string_size - 1, static_cast<unsigned int>(version_string.size()));
	strncpy(dest_string, version_string.c_str(), copy_length);
	*(dest_string + copy_length) = 0x00; //Make sure that the destination string gets null terminated
}

int yafaray_xml_getVersionMajor() { return yafaray_xml::buildinfo::getVersionMajor(); }
int yafaray_xml_getVersionMinor() { return yafaray_xml::buildinfo::getVersionMinor(); }
int yafaray_xml_getVersionPatch() { return yafaray_xml::buildinfo::getVersionPatch(); }
