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
	if(!strcmp(element, "parameters"))
	{
		parser.pushState(startElSurfaceIntegratorParameters, endElSurfaceIntegratorParameters, element, attrs);
	}
	else if(!strcmp(element, "volume_integrator"))
	{
		parser.pushState(startElParamMap, endElParamMap, element, attrs);
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

void startElSurfaceIntegratorParameters(XmlParser &parser, const char *element, const char **attrs)
{
	parseParam(parser.getParamMap(), attrs, element);
}

void endElSurfaceIntegratorParameters(XmlParser &parser, const char *element)
{
	if(strcmp(element, "parameters") == 0)
	{
		parser.createSurfaceIntegrator(parser.stateElementName().c_str());
		parser.popState();
		parser.clearParamMap();
		parser.clearParamMapList();
	}
}

} //namespace yafaray_xml