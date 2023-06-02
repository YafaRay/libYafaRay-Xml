/****************************************************************************
 *
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
 *
 */

#include "import/import_xml.h"
#include "common/version.h"
#include "common/version_build_info.h"
#include <cstring>

namespace yafaray_xml
{

void startElDocument(XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("Document");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);

	if(strcmp(element, "yafaray_xml") == 0)
	{
		if(!attrs || !attrs[0])
		{
			yafaray_printError(parser.getLogger(), "XMLParser: No attributes for yafaray_xml element, cannot check xml format version");
		}
		else if(!strcmp(attrs[0], "format_version"))
		{
			if(attrs[1])
			{
				const std::string format_version_string = attrs[1];
				Version xml_version(format_version_string);
				const int lib_version_major = build_info::getVersionMajor();
				const int lib_version_minor = build_info::getVersionMinor();
				const int lib_version_patch = build_info::getVersionPatch();
				yafaray_printVerbose(parser.getLogger(), ("XMLParser: The XML format version is: '" + xml_version.getString() + "'").c_str());
				if(xml_version.isValid())
				{
					yafaray_printError(parser.getLogger(), ("XMLParser: The XML format version '" + format_version_string + "' is malformed and cannot be checked.").c_str());
				}
				else if((xml_version.getMajor() > lib_version_major) ||
						(xml_version.getMajor() == lib_version_major && xml_version.getMinor() > lib_version_minor) ||
						(xml_version.getMajor() == lib_version_major && xml_version.getMinor() == lib_version_minor && xml_version.getPatch() > lib_version_patch))
				{
					yafaray_printError(parser.getLogger(), ("XMLParser: The XML format version '" + format_version_string + "' is higher than the libYafaRay-Xml version '" + build_info::getVersionString() + "'").c_str());
				}
			}
			else yafaray_printError(parser.getLogger(), "XMLParser: No format version specified for format_version attribute in yafaray_xml element, cannot check xml format version");
		}
		else
		{
			yafaray_printWarning(parser.getLogger(), "XMLParser: Attribute for yafaray_xml element does not match 'format_version'!");
			return;
		}
		parser.pushState(startElYafaRayXml, endElYafaRayXml, "___no_name___");
	}
	else yafaray_printWarning(parser.getLogger(), ("XMLParser: unexpected element <" + std::string(element) + ">, where the element 'yafaray_xml' was expected, skipping...").c_str());
}

void endElDocument(XmlParser &parser, const char *)
{
	yafaray_printVerbose(parser.getLogger(), "XMLParser: Finished document");
}

void startElYafaRayXml(XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("YafarayXml");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);

	if(!strcmp(element, "scene"))
	{
		parser.pushState(startElScene, endElScene, "___no_name___");
	}
	else if(!strcmp(element, "surface_integrator"))
	{
		parser.pushState(startElSurfaceIntegrator, endElSurfaceIntegrator, "___no_name___");
	}
	else if(!strcmp(element, "film"))
	{
		parser.pushState(startElFilm, endElFilm, "___no_name___");
	}
	else yafaray_printWarning(parser.getLogger(), ("XMLParser: Skipping unrecognized YafaRayXml element '" + std::string(element) + "'").c_str());
}

void endElYafaRayXml(XmlParser &parser, const char *element)
{
	if(strcmp(element, "yafaray_xml") == 0)
	{
		parser.popState();
	}
}

} //namespace yafaray_xml