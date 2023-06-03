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
#include "common/vec3f.h"
#include "common/element_parser_utils.h"
#include <cstring>

namespace yafaray_xml
{

static void parsePoint(yafaray_Logger *yafaray_logger, const char **attrs, Vec3f &p, Vec3f &op, int &time_step, bool &has_orco);
static bool parseNormal(yafaray_Logger *yafaray_logger, const char **attrs, Vec3f &n, int &time_step);

void startElObject(XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("Object");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);

	if(!strcmp(element, "p"))
	{
		Vec3f p{0.f, 0.f, 0.f};
		Vec3f op{0.f, 0.f, 0.f};
		int time_step = 0;
		bool has_orco = false;
		parsePoint(parser.getLogger(), attrs, p, op, time_step, has_orco);
		if(has_orco) yafaray_addVertexWithOrcoTimeStep(parser.getScene(), parser.getObjectIdCurrent(), p.x_, p.y_, p.z_, op.x_, op.y_, op.z_, time_step);
		else yafaray_addVertexTimeStep(parser.getScene(), parser.getObjectIdCurrent(), p.x_, p.y_, p.z_, time_step);
	}
	else if(!strcmp(element, "n"))
	{
		Vec3f n(0.0, 0.0, 0.0);
		int time_step = 0;
		if(!parseNormal(parser.getLogger(), attrs, n, time_step)) return;
		yafaray_addNormalTimeStep(parser.getScene(), parser.getObjectIdCurrent(), n.x_, n.y_, n.z_, time_step);
	}
	else if(!strcmp(element, "f"))
	{
		std::vector<int> vertices_indices, uv_indices;
		vertices_indices.reserve(4);
		uv_indices.reserve(4);
		for(; attrs && attrs[0]; attrs += 2)
		{
			const std::string attribute = attrs[0];
			if(attribute.size() == 1) switch(attribute[0])
				{
					case 'a' :
					case 'b' :
					case 'c' :
					case 'd' : vertices_indices.push_back(atoi(attrs[1])); break;
					default: yafaray_printWarning(parser.getLogger(), ("XMLParser: Ignored wrong attribute '" + attribute + "' in face").c_str());
				}
			else
			{
				if(attribute.substr(0, 3) == "uv_") uv_indices.push_back(atoi(attrs[1]));
			}
		}
		if(vertices_indices.size() == 3)
		{
			if(uv_indices.empty()) yafaray_addTriangle(parser.getScene(), parser.getObjectIdCurrent(), vertices_indices[0], vertices_indices[1], vertices_indices[2], parser.getMaterialIdCurrent());
			else yafaray_addTriangleWithUv(parser.getScene(), parser.getObjectIdCurrent(), vertices_indices[0], vertices_indices[1], vertices_indices[2], uv_indices[0], uv_indices[1], uv_indices[2], parser.getMaterialIdCurrent());
		}
		else if(vertices_indices.size() == 4)
		{
			if(uv_indices.empty()) yafaray_addQuad(parser.getScene(), parser.getObjectIdCurrent(), vertices_indices[0], vertices_indices[1], vertices_indices[2], vertices_indices[3], parser.getMaterialIdCurrent());
			else yafaray_addQuadWithUv(parser.getScene(), parser.getObjectIdCurrent(), vertices_indices[0], vertices_indices[1], vertices_indices[2], vertices_indices[3], uv_indices[0], uv_indices[1], uv_indices[2], uv_indices[3], parser.getMaterialIdCurrent());
		}
	}
	else if(!strcmp(element, "uv"))
	{
		float u = 0, v = 0;
		for(; attrs && attrs[0]; attrs += 2)
		{
			switch(attrs[0][0])
			{
				case 'u': u = static_cast<float>(atof(attrs[1]));
					/*if(!(isValid(u)))
					{
						std::cout << std::scientific << std::setprecision(6) << "XMLParser: invalid value in \"" << element << "\" xml entry: " << attrs[0] << "=" << attrs[1] << ". Replacing with 0.0." << std::endl;
						u = 0.f;
					}*/
					break;
				case 'v': v = static_cast<float>(atof(attrs[1]));
					/*	if(!(math::isValid(v)))
						{
							std::cout << std::scientific << std::setprecision(6) << "XMLParser: invalid value in \"" << element << "\" xml entry: " << attrs[0] << "=" << attrs[1] << ". Replacing with 0.0." << std::endl;
							v = 0.f;
						}*/
					break;

				default: yafaray_printWarning(parser.getLogger(), ("XMLParser: Ignored wrong attribute '" + std::string(attrs[0]) + "' in uv").c_str());
			}
		}
		yafaray_addUv(parser.getScene(), parser.getObjectIdCurrent(), u, v);
	}
	else if(!strcmp(element, "set_material"))
	{
		size_t material_id;
		yafaray_getMaterialId(parser.getScene(), &material_id, attrs[1]);
		parser.setMaterialIdCurrent(material_id);
	}
	else if(!strcmp(element, "object_parameters"))
	{
		parser.pushState(startElParammap, endElParammap, getElementName(parser, attrs));
	}
}

void endElObject(XmlParser &parser, const char *element)
{
	if(!strcmp(element, "object"))
	{
		yafaray_initObject(parser.getScene(), parser.getObjectIdCurrent(), parser.getMaterialIdCurrent());
		parser.popState();
	}
}

static void parsePoint(yafaray_Logger *yafaray_logger, const char **attrs, Vec3f &p, Vec3f &op, int &time_step, bool &has_orco)
{
	for(; attrs && attrs[0]; attrs += 2)
	{
		if(attrs[0][0] == 'o')
		{
			has_orco = true;
			if(attrs[0][1] == 0 || attrs[0][2] != 0)
			{
				yafaray_printWarning(yafaray_logger, ("XMLParser: Ignored wrong attribute " + std::string(attrs[0]) + " in orco point (1)").c_str());
				continue; //it is not a single character
			}
			switch(attrs[0][1])
			{
				case 'x' : op.x_ = static_cast<float>(atof(attrs[1])); break;
				case 'y' : op.y_ = static_cast<float>(atof(attrs[1])); break;
				case 'z' : op.z_ = static_cast<float>(atof(attrs[1])); break;
				default: yafaray_printWarning(yafaray_logger, ("XMLParser: Ignored wrong attribute " + std::string(attrs[0]) + " in orco point (2)").c_str());
			}
			continue;
		}
		else if(attrs[0][1] != 0)
		{
			yafaray_printWarning(yafaray_logger, ("XMLParser: Ignored wrong attribute " + std::string(attrs[0]) + " in point").c_str());
			continue; //it is not a single character
		}
		switch(attrs[0][0])
		{
			case 'x' : p.x_ = static_cast<float>(atof(attrs[1])); break;
			case 'y' : p.y_ = static_cast<float>(atof(attrs[1])); break;
			case 'z' : p.z_ = static_cast<float>(atof(attrs[1])); break;
			case 's' : time_step = atoi(attrs[1]); break;
			default: yafaray_printWarning(yafaray_logger, ("XMLParser: Ignored wrong attribute " + std::string(attrs[0]) + " in point").c_str());
		}
	}
}

static bool parseNormal(yafaray_Logger *yafaray_logger, const char **attrs, Vec3f &n, int &time_step)
{
	int number_of_components_read = 0;
	for(; attrs && attrs[0]; attrs += 2)
	{
		if(attrs[0][1] != 0)
		{
			yafaray_printWarning(yafaray_logger, ("XMLParser: Ignored wrong attribute " + std::string(attrs[0]) + " in normal").c_str());
			continue; //it is not a single character
		}
		switch(attrs[0][0])
		{
			case 'x' : n.x_ = static_cast<float>(atof(attrs[1])); ++number_of_components_read; break;
			case 'y' : n.y_ = static_cast<float>(atof(attrs[1])); ++number_of_components_read; break;
			case 'z' : n.z_ = static_cast<float>(atof(attrs[1])); ++number_of_components_read; break;
			case 's' : time_step = atoi(attrs[1]); ++number_of_components_read; break;
			default: yafaray_printWarning(yafaray_logger, ("XMLParser: Ignored wrong attribute " + std::string(attrs[0]) + " in normal").c_str());
		}
	}
	return (number_of_components_read == 3 || number_of_components_read == 4);
}

} //namespace yafaray_xml