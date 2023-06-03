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
#include "common/element_parser_utils.h"
#include <cstring>

namespace yafaray_xml
{

void startElFilm(XmlParser &parser, const char *element, const char **attrs)
{

	if(!strcmp(element, "film_parameters") || !strcmp(element, "camera") || !strcmp(element, "output") || !strcmp(element, "layer"))
	{
		parser.pushState(startElParammap, endElParammap, element, attrs);
	}
	else yafaray_printWarning(parser.getLogger(), ("XMLParser: Skipping unrecognized element '" + std::string(element) + "'").c_str());
}

void endElFilm(XmlParser &parser, const char *element)
{
	if(strcmp(element, "film") == 0)
	{
		parser.popState();
	}
}

} //namespace yafaray_xml