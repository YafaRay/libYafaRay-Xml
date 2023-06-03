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
#include "common/get_element_name.h"
#include <cstring>

namespace yafaray_xml
{

void startElScene(XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("Scene");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);

	if(!strcmp(element, "scene_parameters") || !strcmp(element, "material") || !strcmp(element, "light") || !strcmp(element, "texture") || !strcmp(element, "volumeregion") || !strcmp(element, "image") || !strcmp(element, "light"))
	{
		parser.pushState(startElParammap, endElParammap, getElementName(parser, attrs));
	}
	else if(!strcmp(element, "background"))
	{
		parser.pushState(startElParammap, endElParammap, getElementName(parser, attrs));
	}
	else if(!strcmp(element, "object"))
	{
		parser.pushState(startElObject, endElObject, "");
	}
	else if(!strcmp(element, "smooth"))
	{
		double angle = 181.0;
		std::string element_name;
		for(int n = 0; attrs[n]; ++n)
		{
			if(!strcmp(attrs[n], "object_name")) element_name = attrs[n + 1];
			else if(!strcmp(attrs[n],"angle")) angle = atof(attrs[n + 1]);
		}
		//not optimal to take ID blind...
		//yafaray_startObjects();
		size_t object_id;
		yafaray_getObjectId(parser.getScene(), &object_id, element_name.c_str());
		bool success = yafaray_smoothObjectMesh(parser.getScene(), object_id, angle);
		if(!success) yafaray_printWarning(parser.getLogger(), ("XMLParser: Couldn't smooth object with object_name='" + element_name + "', angle = " + std::to_string(angle)).c_str());
		//yafaray_endObjects();
		parser.pushState(startElDummy, endElDummy, getElementName(parser, attrs));
	}
	else if(!strcmp(element, "accelerator"))
	{
		parser.pushState(startElParammap, endElParammap, getElementName(parser, attrs));
	}
	else if(!strcmp(element, "createInstance"))
	{
		parser.setInstanceIdCurrent(yafaray_createInstance(parser.getScene()));
		parser.pushState(nullptr, endElCreateInstance, "");
	}
	else if(!strcmp(element, "addInstanceObject"))
	{
		std::string base_object_name;
		for(int n = 0; attrs[n]; n++)
		{
			if(!strcmp(attrs[n], "instance_id"))
			{
				parser.setInstanceIdCurrent(atoi(attrs[n + 1]));
			}
			else if(!strcmp(attrs[n], "base_object_name"))
			{
				base_object_name = attrs[n + 1];
			}
		}
		size_t object_id;
		yafaray_getObjectId(parser.getScene(), &object_id, base_object_name.c_str());
		yafaray_addInstanceObject(parser.getScene(), parser.getInstanceIdCurrent(), object_id);
		parser.pushState(nullptr, endElAddInstanceObject, "");
	}
	else if(!strcmp(element, "addInstanceOfInstance"))
	{
		unsigned int instance_id = -1;
		unsigned int base_instance_id = -1;
		for(int n = 0; attrs[n]; n++)
		{
			if(!strcmp(attrs[n], "instance_id"))
			{
				instance_id = atoi(attrs[n + 1]);
			}
			else if(!strcmp(attrs[n], "base_instance_id"))
			{
				base_instance_id = atoi(attrs[n + 1]);
			}
		}
		yafaray_addInstanceOfInstance(parser.getScene(), instance_id, base_instance_id);
		parser.pushState(nullptr, endElAddInstanceOfInstance, "");
	}
	else if(!strcmp(element, "addInstanceMatrix"))
	{
		for(int n = 0; attrs[n]; n++)
		{
			if(!strcmp(attrs[n], "instance_id"))
			{
				parser.setInstanceIdCurrent(atoi(attrs[n + 1]));
			}
			else if(!strcmp(attrs[n], "time")) parser.setTimeCurrent(static_cast<float>(atof(attrs[n + 1])));
		}
		parser.pushState(startElAddInstanceMatrix, endElAddInstanceMatrix, "");
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

} //namespace yafaray_xml