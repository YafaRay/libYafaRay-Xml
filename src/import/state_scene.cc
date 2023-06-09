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

void startElScene(XmlParser &parser, const char *element, const char **attrs)
{
	if(!strcmp(element, "parameters"))
	{
		parser.pushState(startElSceneParameters, endElSceneParameters, element, attrs);
	}
	else if(!strcmp(element, "accelerator") || !strcmp(element, "material") || !strcmp(element, "light") || !strcmp(element, "texture") || !strcmp(element, "volume_region") || !strcmp(element, "image") || !strcmp(element, "light") || !strcmp(element, "background"))
	{
		parser.pushState(startElParamMap, endElParamMap, element, attrs);
	}
	else if(!strcmp(element, "object"))
	{
		parser.pushState(startElObject, endElObject, element, attrs);
	}
	else if(!strcmp(element, "instance"))
	{
		parser.setInstanceIdCurrent(yafaray_createInstance(parser.getScene()));
		parser.pushState(startElInstance, endElInstance, element, attrs);
	}
	else if(!strcmp(element, "smooth"))
	{
		double angle = 181.0;
		std::string object_name;
		for(int n = 0; attrs[n]; ++n)
		{
			if(!strcmp(attrs[n], "object_name")) object_name = attrs[n + 1];
			else if(!strcmp(attrs[n],"angle")) angle = atof(attrs[n + 1]);
		}
		size_t object_id;
		yafaray_getObjectId(parser.getScene(), &object_id, object_name.c_str());
		bool success = yafaray_smoothObjectMesh(parser.getScene(), object_id, angle);
		if(!success) yafaray_printWarning(parser.getLogger(), ("XMLParser: Couldn't smooth object with object_name='" + object_name + "', angle = " + std::to_string(angle)).c_str());
		parser.pushState(startElSmooth, endElSmooth, element, attrs);
	}
	else yafaray_printWarning(parser.getLogger(), ("XMLParser: Skipping unrecognized element '" + std::string(element) + "'").c_str());
}

void endElScene(XmlParser &parser, const char *element)
{
	if(strcmp(element, "scene") == 0)
	{
		parser.popState();
	}
}

void startElSceneParameters(XmlParser &parser, const char *element, const char **attrs)
{
	parseParam(parser.getParamMap(), attrs, element);
}

void endElSceneParameters(XmlParser &parser, const char *element)
{
	if(strcmp(element, "parameters") == 0)
	{
		parser.createScene(parser.stateElementName().c_str());
		parser.popState();
		parser.clearParamMap();
		parser.clearParamMapList();
	}
}

} //namespace yafaray_xml