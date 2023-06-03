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
#include "common/rgba.h"
#include <cstring>

namespace yafaray_xml
{

void parseParam(yafaray_ParamMap *yafaray_param_map, const char **attrs, const char *param_name)
{
	if(!attrs || !attrs[0]) return;
	if(!attrs[2]) // only one attribute => bool, integer or float value
	{
		if(!strcmp(attrs[0], "ival"))
		{
			const int i = atoi(attrs[1]);
			yafaray_setParamMapInt(yafaray_param_map, param_name, i);
			return;
		}
		else if(!strcmp(attrs[0], "fval"))
		{
			const double f = atof(attrs[1]);
			yafaray_setParamMapFloat(yafaray_param_map, param_name, f);
			return;
		}
		else if(!strcmp(attrs[0], "bval"))
		{
			const bool b = strcmp(attrs[1], "true") == 0;
			yafaray_setParamMapBool(yafaray_param_map, param_name, static_cast<yafaray_Bool>(b));
			return;
		}
		else if(!strcmp(attrs[0], "sval"))
		{
			yafaray_setParamMapString(yafaray_param_map, param_name, attrs[1]);
			return;
		}
	}
	Rgba c(0.f);
	Vec3f v(0, 0, 0);
	double matrix[4][4];
	enum class ParameterType : int { None, Vector, Color, Matrix } type = ParameterType::None;
	for(int n = 0; attrs[n]; ++n)
	{
		if(attrs[n][1] == '\0')
		{
			switch(attrs[n][0])
			{
				case 'x': v.x_ = static_cast<float>(atof(attrs[n + 1])); type = ParameterType::Vector; break;
				case 'y': v.y_ = static_cast<float>(atof(attrs[n + 1])); type = ParameterType::Vector; break;
				case 'z': v.z_ = static_cast<float>(atof(attrs[n + 1])); type = ParameterType::Vector; break;

				case 'r': c.r_ = static_cast<float>(atof(attrs[n + 1])); type = ParameterType::Color; break;
				case 'g': c.g_ = static_cast<float>(atof(attrs[n + 1])); type = ParameterType::Color; break;
				case 'b': c.b_ = static_cast<float>(atof(attrs[n + 1])); type = ParameterType::Color; break;
				case 'a': c.a_ = static_cast<float>(atof(attrs[n + 1])); type = ParameterType::Color; break;
			}
		}
		else if(attrs[n][3] == '\0' && attrs[n][0] == 'm' && attrs[n][1] >= '0' && attrs[n][1] <= '3' && attrs[n][2] >= '0' && attrs[n][2] <= '3') //"mij" where i and j are between 0 and 3 (inclusive)
		{
			type = ParameterType::Matrix;
			const int i = attrs[n][1] - '0';
			const int j = attrs[n][2] - '0';
			matrix[i][j] = atof(attrs[n + 1]);
		}
	}

	if(type == ParameterType::Vector) yafaray_setParamMapVector(yafaray_param_map, param_name, v.x_, v.y_, v.z_);
	else if(type == ParameterType::Matrix) yafaray_setParamMapMatrixArray(yafaray_param_map, param_name, reinterpret_cast<const double *>(matrix), static_cast<yafaray_Bool>(false));
	else if(type == ParameterType::Color)
	{
		yafaray_setParamMapColor(yafaray_param_map, param_name, c.r_, c.g_, c.b_, c.a_);
	}
}

} //namespace yafaray_xml