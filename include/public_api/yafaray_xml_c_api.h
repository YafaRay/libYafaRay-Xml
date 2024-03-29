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

#ifndef LIBYAFARAY_XML_C_API_H
#define LIBYAFARAY_XML_C_API_H

#include "yafaray_xml_c_api_export.h"
#include <yafaray_c_api.h>

#define YAFARAY_XML_C_API_VERSION_MAJOR 4

#ifdef __cplusplus
extern "C" {
#endif

	YAFARAY_XML_C_API_EXPORT yafaray_Container *yafaray_xml_ParseFile(yafaray_Logger *yafaray_logger, const char *xml_file_path, const char *input_color_space, float input_gamma);
	YAFARAY_XML_C_API_EXPORT yafaray_Container *yafaray_xml_ParseMemory(yafaray_Logger *yafaray_logger, const char *xml_buffer, int xml_buffer_size, const char *input_color_space, float input_gamma);
	YAFARAY_XML_C_API_EXPORT int yafaray_xml_getVersionMajor();
	YAFARAY_XML_C_API_EXPORT int yafaray_xml_getVersionMinor();
	YAFARAY_XML_C_API_EXPORT int yafaray_xml_getVersionPatch();
	/* The following functions return a text string where memory is allocated by libYafaRay itself. Do not free the char* directly with free, use "yafaray_xml_destroyCharString" to free them instead to ensure proper deallocation. */
	YAFARAY_XML_C_API_EXPORT char *yafaray_xml_getVersionString();
	YAFARAY_XML_C_API_EXPORT void yafaray_xml_destroyCharString(char *string);

#ifdef __cplusplus
}
#endif

#endif /* LIBYAFARAY_XML_C_API_H */
