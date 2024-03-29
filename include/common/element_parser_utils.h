#pragma once
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

#ifndef LIBYAFARAY_XML_ELEMENT_PARSER_UTILS_H
#define LIBYAFARAY_XML_ELEMENT_PARSER_UTILS_H

#include "import/import_xml.h"
#include <cstring>
#include <string>
#include <sstream>

namespace yafaray_xml
{

inline std::string getElementName(XmlParser &parser, const char **attrs)
{
	if(!attrs || !attrs[0])
	{
		//yafaray_printWarning(parser.getLogger(), ("XMLParser: No attributes for element '" + element_name + "'!").c_str());
		return {};
	}

	if(attrs && !strcmp(attrs[0], "name"))
	{
		return attrs[1];
	}
	else
	{
		//yafaray_printWarning(parser.getLogger(), ("XMLParser: Attribute for element '" + element_name + "does not match 'name'!").c_str());
		return {};
	}
}

inline std::string getElementAttrs(const char **attrs)
{
	if(attrs)
	{
		std::stringstream ss;
		ss << " ";
		for(size_t i = 0; attrs[2 * i]; ++i)
		{
			if(i > 0) ss << " ";
			ss << attrs[2 * i];
			if(attrs[2 * i + 1]) ss << "=\"" << attrs[2 * i + 1];
			ss << "\"";
		}
		return ss.str();
	}
	else
	{
		return {};
	}
}

} //namespace yafaray_xml

#endif //LIBYAFARAY_XML_ELEMENT_PARSER_UTILS_H
