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

void startElInstance(XmlParser &parser, const char *element, const char **attrs)
{
	if(!strcmp(element, "object_ref"))
	{
		std::string object_name;
		for(int n = 0; attrs[n]; n++)
		{
			if(!strcmp(attrs[n], "name"))
			{
				object_name = attrs[n + 1];
			}
		}
		size_t object_id;
		yafaray_getObjectId(parser.getScene(), &object_id, object_name.c_str());
		yafaray_addInstanceObject(parser.getScene(), parser.getInstanceIdCurrent(), object_id);
	}
	else if(!strcmp(element, "instance_ref"))
	{
		unsigned int base_instance_id = -1;
		for(int n = 0; attrs[n]; n++)
		{
			if(!strcmp(attrs[n], "id"))
			{
				base_instance_id = atoi(attrs[n + 1]);
			}
		}
		yafaray_addInstanceOfInstance(parser.getScene(), parser.getInstanceIdCurrent(), base_instance_id);
	}
	else if(!strcmp(element, "matrix"))
	{
		float time{0.f};
		double m[4 * 4];
		for(int n = 0; attrs[n]; ++n)
		{
			if(attrs[n][0] == 't')
			{
				time = static_cast<float>(atof(attrs[n + 1]));
			}
			if(attrs[n][3] == '\0' && attrs[n][0] == 'm' && attrs[n][1] >= '0' && attrs[n][1] <= '3' && attrs[n][2] >= '0' && attrs[n][2] <= '3') //"mij" where i and j are between 0 and 3 (inclusive)
			{
				const int i = attrs[n][1] - '0';
				const int j = attrs[n][2] - '0';
				m[4 * i + j] = atof(attrs[n + 1]);
			}
		}
		yafaray_addInstanceMatrixArray(parser.getScene(), parser.getInstanceIdCurrent(), m, time);
	}
}

void endElInstance(XmlParser &parser, const char *element)
{
	if(!strcmp(element, "instance"))
	{
		parser.popState();
	}
}

} //namespace yafaray_xml