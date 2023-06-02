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
#include <cstring>

namespace yafaray_xml
{

void startElSurfaceIntegrator(XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("SurfaceIntegrator");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);

	if(!strcmp(element, "surface_integrator_parameters"))
	{
		std::string element_name;
		if(!attrs || !attrs[0])
		{
			yafaray_printWarning(parser.getLogger(), ("XMLParser: No attributes for element '" + element_name + "'!").c_str());
			return;
		}
		else if(!strcmp(attrs[0], "name")) element_name = attrs[1];
		else
		{
			yafaray_printWarning(parser.getLogger(), ("XMLParser: Attribute for element '" + element_name + "does not match 'name'!").c_str());
			return;
		}
		parser.pushState(startElParammap, endElParammap, element_name);
	}
	else if(!strcmp(element, "surface_integrator") || !strcmp(element, "volume_integrator"))
	{
		parser.pushState(startElParammap, endElParammap, "___no_name___");
	}
	else yafaray_printWarning(parser.getLogger(), ("XMLParser: Skipping unrecognized element '" + std::string(element) + "'").c_str());
}

void endElSurfaceIntegrator(XmlParser &parser, const char *element)
{
	if(strcmp(element, "surface_integrator") == 0)
	{
		parser.popState();
	}
}

} //namespace yafaray_xml