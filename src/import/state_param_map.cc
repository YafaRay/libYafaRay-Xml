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

void startElParammap(XmlParser &parser, const char *element, const char **attrs)
{
	if(!strcmp(element, "shader_node"))
	{
		parser.pushState(startElShaderNode, endElShaderNode, element, attrs);
		return;
	}
	parseParam(parser.getParamMap(), attrs, element);
}

void endElParammap(XmlParser &parser, const char *element)
{
	//yafaray_printDebug(parser.getLogger(), parser.getScene(), parser.getRenderer(), parser.getFilm(), ("##### endElParammap, element='" + std::string(element) + "', element_name='" + std::string(parser.stateElementName()) + "'").c_str());
	const bool exit_state = (parser.currLevel() == parser.stateLevel());
	if(exit_state)
	{
		const std::string element_name = parser.stateElementName();
		if(element_name.empty() && strcmp(element, "createInstance") != 0 && strcmp(element, "addInstanceObject") != 0 && strcmp(element, "addInstanceOfInstance") != 0 && strcmp(element, "addInstanceMatrix") != 0 && strcmp(element, "background") != 0 && strcmp(element, "surface_integrator") != 0 && strcmp(element, "volume_integrator") != 0 && strcmp(element, "layer") != 0 && strcmp(element, "accelerator") != 0)
		{
			yafaray_printWarning(parser.getLogger(), ("XMLParser: No name for element '" + std::string(element) + "' available!").c_str());
		}
		else
		{
			if(!strcmp(element, "scene_parameters")) parser.createScene(element_name.c_str());
			else if(!strcmp(element, "surface_integrator_parameters")) parser.createSurfaceIntegrator(element_name.c_str());
			else if(!strcmp(element, "film_parameters")) parser.createFilm(element_name.c_str());
			else if(!strcmp(element, "material"))
			{
				size_t material_id;
				yafaray_createMaterial(parser.getScene(), &material_id, element_name.c_str(), parser.getParamMap(), parser.getParamMapList());
				parser.setMaterialIdCurrent(material_id);
			}
			else if(!strcmp(element, "volume_integrator")) yafaray_defineVolumeIntegrator(parser.getSurfaceIntegrator(), parser.getScene(), parser.getParamMap());
			else if(!strcmp(element, "light")) yafaray_createLight(parser.getScene(), element_name.c_str(), parser.getParamMap());
			else if(!strcmp(element, "image"))
				yafaray_createImage(parser.getScene(), element_name.c_str(), nullptr, parser.getParamMap());
			else if(!strcmp(element, "texture")) yafaray_createTexture(parser.getScene(), element_name.c_str(), parser.getParamMap());
			else if(!strcmp(element, "camera")) yafaray_defineCamera(parser.getFilm(), element_name.c_str(), parser.getParamMap());
			else if(!strcmp(element, "accelerator")) yafaray_setSceneAcceleratorParams(parser.getScene(), parser.getParamMap());
			else if(!strcmp(element, "background")) yafaray_defineBackground(parser.getScene(), parser.getParamMap());
			else if(!strcmp(element, "object_parameters"))
			{
				size_t object_id;
				yafaray_createObject(parser.getScene(), &object_id, element_name.c_str(), parser.getParamMap());
				parser.setObjectIdCurrent(object_id);
			}
			else if(!strcmp(element, "volumeregion")) yafaray_createVolumeRegion(parser.getScene(), element_name.c_str(), parser.getParamMap());
			else if(!strcmp(element, "layer")) { yafaray_defineLayer(parser.getFilm(), parser.getParamMap()); }
			else if(!strcmp(element, "output")) yafaray_createOutput(parser.getFilm(), element_name.c_str(), parser.getParamMap());
			else yafaray_printWarning(parser.getLogger(), ("XMLParser: Unexpected end-tag of element '" + std::string(element) + "'!").c_str());
		}
		parser.popState();
		parser.clearParamMap();
		parser.clearParamMapList();
	}
}

} //namespace yafaray_xml