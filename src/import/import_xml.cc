/****************************************************************************
 *      xmlparser.cc: a libXML based parser for YafRay scenes
 *      This is part of the libYafaRay-Xml package
 *      Copyright (C) 2006  Mathias Wein
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
 */

#include "import/import_xml.h"
#include "common/version_build_info.h"
#include <libxml/parser.h>
#include <sstream>
#include <cstring>
#include <array>

BEGIN_YAFARAY_XML

void XmlParser::setLastElementName(const char *element_name)
{
	if(element_name) current_->last_element_ = element_name;
	else current_->last_element_.clear();
}

void XmlParser::setLastElementNameAttrs(const char **element_attrs)
{
	current_->last_element_attrs_.clear();
	if(element_attrs)
	{
		for(int n = 0; element_attrs[n]; ++n)
		{
			if(n > 0) current_->last_element_attrs_ += " ";
			current_->last_element_attrs_ += element_attrs[n];
		}
	}
}

void startDocument_global(void *)
{
	//Empty
}

void endDocument_global(void *)
{
	//Empty
}

void startElement_global(void *user_data, const xmlChar *name, const xmlChar **attrs)
{
	XmlParser &parser = *static_cast<XmlParser *>(user_data);
	parser.startElement(parser.getInterface(), (const char *)name, (const char **)attrs);
}

void endElement_global(void *user_data, const xmlChar *name)
{
	XmlParser &parser = *static_cast<XmlParser *>(user_data);
	parser.endElement(parser.getInterface(), (const char *)name);
}

enum XmlErrorSeverity { Warning, Error, FatalError };
static void xmlErrorProcessing_global(XmlErrorSeverity xml_error_severity, void *user_data, const char *msg, ...)
{
	XmlParser &parser = *static_cast<XmlParser *>(user_data);
	const size_t message_size = 1000;
	char message_buffer[message_size];
	va_list args;
	va_start(args, msg);
	vsnprintf(message_buffer, message_size, msg, args);
	va_end(args);
	std::stringstream message_stream;
	message_stream << "XMLParser ";
	switch(xml_error_severity)
	{
		case XmlErrorSeverity::FatalError: message_stream << "fatal error: "; break;
		case XmlErrorSeverity::Error: message_stream << "error: "; break;
		case XmlErrorSeverity::Warning:
		default: message_stream << "warning: "; break;
	}
	message_stream << "XMLParser warning: " << std::string(message_buffer);
	message_stream << " in section '" << parser.getLastSection() << ", level " << parser.currLevel();
	message_stream << " an element previous to the error: '" << parser.getLastElementName() << "', attrs: { " << parser.getLastElementNameAttrs() << " }";
	yafaray_Interface_t *yafaray_interface = parser.getInterface();
	switch(xml_error_severity)
	{
		case XmlErrorSeverity::FatalError:
		case XmlErrorSeverity::Error: yafaray_printError(yafaray_interface, message_stream.str().c_str()); break;
		case XmlErrorSeverity::Warning:
		default: yafaray_printWarning(yafaray_interface, message_stream.str().c_str()); break;
	}
}

static void myWarning_global(void *user_data, const char *msg, ...)
{
	va_list args;
	xmlErrorProcessing_global(XmlErrorSeverity::Warning, user_data, msg, args);
}

static void myError_global(void *user_data, const char *msg, ...)
{
	va_list args;
	xmlErrorProcessing_global(XmlErrorSeverity::Error, user_data, msg, args);
}

static void myFatalError_global(void *user_data, const char *msg, ...)
{
	va_list args;
	xmlErrorProcessing_global(XmlErrorSeverity::FatalError, user_data, msg, args);
	va_end(args);
}

static xmlSAXHandler my_handler_global =
{
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	startDocument_global, //  startDocumentSAXFunc startDocument;
	endDocument_global, //  endDocumentSAXFunc endDocument;
	startElement_global, //  startElementSAXFunc startElement;
	endElement_global, //  endElementSAXFunc endElement;
	nullptr,
	nullptr, //  charactersSAXFunc characters;
	nullptr,
	nullptr,
	nullptr,
	myWarning_global,
	myError_global,
	myFatalError_global
};

bool XmlParser::parseXmlFile(yafaray_Interface_t *yafaray_interface, const char *xml_file_path) noexcept
{
	XmlParser parser(yafaray_interface);
	if(!xml_file_path || xmlSAXUserParseFile(&my_handler_global, &parser, xml_file_path) < 0)
	{
		yafaray_printError(yafaray_interface, ("XMLParser: Error parsing the file " + std::string(xml_file_path)).c_str());
		return false;
	}
	return true;
}

bool XmlParser::parseXmlMemory(yafaray_Interface_t *yafaray_interface, const char *xml_buffer, unsigned int xml_buffer_size) noexcept
{
	const int xml_buffer_size_signed = static_cast<int>(xml_buffer_size);
	XmlParser parser(yafaray_interface);
	if(!xml_buffer || xml_buffer_size_signed <= 0 || xmlSAXUserParseMemory(&my_handler_global, &parser, xml_buffer, xml_buffer_size_signed) < 0)
	{
		yafaray_printError(yafaray_interface, "XMLParser: Error parsing a memory buffer");
		return false;
	}
	return true;
}

/*=============================================================
/ parser functions
=============================================================*/

XmlParser::XmlParser(yafaray_Interface_t *yafaray_interface):
		yafaray_interface_(yafaray_interface)
{
	pushState(startElDocument_global, endElDocument_global, "");
}

void XmlParser::pushState(StartElementCb_t start, EndElementCb_t end, const std::string &element_name)
{
	ParserState state;
	state.start_ = start;
	state.end_ = end;
	state.element_name_ = element_name;
	state.level_ = level_;
	state_stack_.push_back(state);
	current_ = &state_stack_.back();
}

void XmlParser::popState()
{
	state_stack_.pop_back();
	if(!state_stack_.empty()) current_ = &state_stack_.back();
	else current_ = nullptr;
}

/*=============================================================
/ utility functions...
=============================================================*/

inline bool str2Bool_global(const char *s) { return strcmp(s, "true") == 0; }

static void parsePoint_global(yafaray_Interface_t *yafaray_interface, const char **attrs, Vec3f &p, Vec3f &op, bool &has_orco)
{
	for(; attrs && attrs[0]; attrs += 2)
	{
		if(attrs[0][0] == 'o')
		{
			has_orco = true;
			if(attrs[0][1] == 0 || attrs[0][2] != 0)
			{
				yafaray_printWarning(yafaray_interface, ("XMLParser: Ignored wrong attribute " + std::string(attrs[0]) + " in orco point (1)").c_str());
				continue; //it is not a single character
			}
			switch(attrs[0][1])
			{
				case 'x' : op.x_ = atof(attrs[1]); break;
				case 'y' : op.y_ = atof(attrs[1]); break;
				case 'z' : op.z_ = atof(attrs[1]); break;
				default: yafaray_printWarning(yafaray_interface, ("XMLParser: Ignored wrong attribute " + std::string(attrs[0]) + " in orco point (2)").c_str());
			}
			continue;
		}
		else if(attrs[0][1] != 0)
		{
			yafaray_printWarning(yafaray_interface, ("XMLParser: Ignored wrong attribute " + std::string(attrs[0]) + " in point").c_str());
			continue; //it is not a single character
		}
		switch(attrs[0][0])
		{
			case 'x' : p.x_ = atof(attrs[1]); break;
			case 'y' : p.y_ = atof(attrs[1]); break;
			case 'z' : p.z_ = atof(attrs[1]); break;
			default: yafaray_printWarning(yafaray_interface, ("XMLParser: Ignored wrong attribute " + std::string(attrs[0]) + " in point").c_str());
		}
	}
}

static bool parseNormal_global(yafaray_Interface_t *yafaray_interface, const char **attrs, Vec3f &n)
{
	int compo_read = 0;
	for(; attrs && attrs[0]; attrs += 2)
	{
		if(attrs[0][1] != 0)
		{
			yafaray_printWarning(yafaray_interface, ("XMLParser: Ignored wrong attribute " + std::string(attrs[0]) + " in normal").c_str());
			continue; //it is not a single character
		}
		switch(attrs[0][0])
		{
			case 'x' : n.x_ = atof(attrs[1]); compo_read++; break;
			case 'y' : n.y_ = atof(attrs[1]); compo_read++; break;
			case 'z' : n.z_ = atof(attrs[1]); compo_read++; break;
			default: yafaray_printWarning(yafaray_interface, ("XMLParser: Ignored wrong attribute " + std::string(attrs[0]) + " in normal").c_str());
		}
	}
	return (compo_read == 3);
}

void parseParam_global(yafaray_Interface_t *yafaray_interface, const char **attrs, const char *param_name, XmlParser &parser)
{
	if(!attrs || !attrs[0]) return;
	if(!attrs[2]) // only one attribute => bool, integer or float value
	{
		if(!strcmp(attrs[0], "ival")) { const int i = atoi(attrs[1]); yafaray_paramsSetInt(yafaray_interface, param_name, i); return; }
		else if(!strcmp(attrs[0], "fval")) { const double f = atof(attrs[1]); yafaray_paramsSetFloat(yafaray_interface, param_name, f); return; }
		else if(!strcmp(attrs[0], "bval")) { const bool b = str2Bool_global(attrs[1]); yafaray_paramsSetBool(yafaray_interface, param_name, static_cast<yafaray_bool_t>(b)); return; }
		else if(!strcmp(attrs[0], "sval")) { yafaray_paramsSetString(yafaray_interface, param_name, attrs[1]); return; }
	}
	Rgba c(0.f);
	Vec3f v(0, 0, 0);
	std::array<std::array<float, 4>, 4> m;
	enum class ParameterType : int { None, Vector, Color, Matrix } type = ParameterType::None;
	for(int n = 0; attrs[n]; ++n)
	{
		if(attrs[n][1] == '\0')
		{
			switch(attrs[n][0])
			{
				case 'x': v.x_ = atof(attrs[n + 1]); type = ParameterType::Vector; break;
				case 'y': v.y_ = atof(attrs[n + 1]); type = ParameterType::Vector; break;
				case 'z': v.z_ = atof(attrs[n + 1]); type = ParameterType::Vector; break;

				case 'r': c.r_ = atof(attrs[n + 1]); type = ParameterType::Color; break;
				case 'g': c.g_ = atof(attrs[n + 1]); type = ParameterType::Color; break;
				case 'b': c.b_ = atof(attrs[n + 1]); type = ParameterType::Color; break;
				case 'a': c.a_ = atof(attrs[n + 1]); type = ParameterType::Color; break;
			}
		}
		else if(attrs[n][3] == '\0' && attrs[n][0] == 'm' && attrs[n][1] >= '0' && attrs[n][1] <= '3' && attrs[n][2] >= '0' && attrs[n][2] <= '3') //"mij" where i and j are between 0 and 3 (inclusive)
		{
			type = ParameterType::Matrix;
			const int i = attrs[n][1] - '0';
			const int j = attrs[n][2] - '0';
			m[i][j] = atof(attrs[n + 1]);
		}
	}

	if(type == ParameterType::Vector) yafaray_paramsSetVector(yafaray_interface, param_name, v.x_, v.y_, v.z_);
	else if(type == ParameterType::Matrix) yafaray_paramsSetMatrix(yafaray_interface, param_name, m.data()->data(), static_cast<yafaray_bool_t>(false));
	else if(type == ParameterType::Color)
	{
		yafaray_paramsSetColor(yafaray_interface, param_name, c.r_, c.g_, c.b_, c.a_);
	}
}

/*=============================================================
/ start- and endElement callbacks for the different states
=============================================================*/

void endElDummy_global(yafaray_Interface_t *yafaray_interface, XmlParser &parser, const char *)
{
	parser.popState();
}

void startElDummy_global(yafaray_Interface_t *yafaray_interface, XmlParser &parser, const char *, const char **)
{
	parser.pushState(startElDummy_global, endElDummy_global, "___no_name___");
}

class Version final
{
	public:
		explicit Version(const std::string &version_string);
		std::string getString() const { return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch); }
		int getMajor() const { return major; }
		int getMinor() const { return minor; }
		int getPatch() const { return patch; }
		bool isValid() const { return major == 0 && minor == 0 && patch == 0; }
	private:
		int major = 0;
		int minor = 0;
		int patch = 0;
};

Version::Version(const std::string &version_string)
{
	bool error = false;
	size_t pos_first_dot = version_string.find('.');
	if(pos_first_dot < version_string.size())
	{
		major = std::stoi(version_string.substr(0, pos_first_dot));
		std::string remaining_string = version_string.substr(pos_first_dot + 1);
		size_t pos_second_dot = remaining_string.find('.');
		if(pos_second_dot < remaining_string.size())
		{
			minor = std::stoi(remaining_string.substr(0, pos_second_dot));
			patch = std::stoi(remaining_string.substr(pos_second_dot + 1));
		}
		else error = true;
	}
	else error = true;

	if(error)
	{
		major = 0;
		minor = 0;
		patch = 0;
	}
}

void startElDocument_global(yafaray_Interface_t *yafaray_interface, XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("Document");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);

	if(strcmp(element, "yafaray_xml") == 0)
	{
		if(!attrs || !attrs[0])
		{
			yafaray_printError(yafaray_interface, "XMLParser: No attributes for yafaray_xml element, cannot check xml format version");
		}
		else if(!strcmp(attrs[0], "format_version"))
		{
			if(attrs[1])
			{
				const std::string format_version_string = attrs[1];
				Version xml_version(format_version_string);
				const int lib_version_major = buildinfo::getVersionMajor();
				const int lib_version_minor = buildinfo::getVersionMinor();
				const int lib_version_patch = buildinfo::getVersionPatch();
				yafaray_printVerbose(yafaray_interface, ("XMLParser: The XML format version is: '" + xml_version.getString() + "'").c_str());
				if(xml_version.isValid())
				{
					yafaray_printError(yafaray_interface, ("XMLParser: The XML format version '" + format_version_string + "' is malformed and cannot be checked.").c_str());
				}
				else if((xml_version.getMajor() > lib_version_major) ||
						(xml_version.getMajor() == lib_version_major && xml_version.getMinor() > lib_version_minor) ||
						(xml_version.getMajor() == lib_version_major && xml_version.getMinor() == lib_version_minor && xml_version.getPatch() > lib_version_patch))
				{
					yafaray_printError(yafaray_interface, ("XMLParser: The XML format version '" + format_version_string + "' is higher than the libYafaRay-Xml version '" + buildinfo::getVersionString() + "'").c_str());
				}
			}
			else yafaray_printError(yafaray_interface, "XMLParser: No format version specified for format_version attribute in yafaray_xml element, cannot check xml format version");
		}
		else
		{
			yafaray_printWarning(yafaray_interface, "XMLParser: Attribute for yafaray_xml element does not match 'format_version'!");
			return;
		}
		parser.pushState(startElYafaRayXml_global, endElYafaRayXml_global, "___no_name___");
	}
	else yafaray_printWarning(yafaray_interface, ("XMLParser: unexpected element <" + std::string(element) + ">, where the element 'yafaray_xml' was expected, skipping...").c_str());
}

void endElDocument_global(yafaray_Interface_t *yafaray_interface, XmlParser &parser, const char *)
{
	yafaray_printVerbose(yafaray_interface, "XMLParser: Finished document");
}

// scene-state, i.e. expect only primary elements
// such as light, material, texture, object, integrator, render...

void startElYafaRayXml_global(yafaray_Interface_t *yafaray_interface, XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("YafarayXml");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);

	if(!strcmp(element, "material") || !strcmp(element, "integrator") || !strcmp(element, "light") || !strcmp(element, "texture") || !strcmp(element, "camera") || !strcmp(element, "background") || !strcmp(element, "volumeregion") || !strcmp(element, "logging_badge") || !strcmp(element, "output") || !strcmp(element, "render_view"))
	{
		std::string element_name;
		if(!attrs || !attrs[0])
		{
			yafaray_printWarning(yafaray_interface, ("XMLParser: No attributes for scene element '" + element_name + "'!").c_str());
			return;
		}
		else if(!strcmp(attrs[0], "name")) element_name = attrs[1];
		else
		{
			yafaray_printWarning(yafaray_interface, ("XMLParser: Attribute for scene element '" + element_name + "does not match 'name'!").c_str());
			return;
		}
		parser.pushState(startElParammap_global, endElParammap_global, element_name);
	}
	else if(!strcmp(element, "layer") || !strcmp(element, "layers_parameters") || !strcmp(element, "scene") || !strcmp(element, "render"))
	{
		parser.pushState(startElParammap_global, endElParammap_global, "___no_name___");
	}
	else if(!strcmp(element, "object"))
	{
		const std::string element_name = "Object_" + std::to_string(yafaray_getNextFreeId(yafaray_interface));
		parser.pushState(startElObject_global, endElObject_global, element_name);
	}
	else if(!strcmp(element, "smooth"))
	{
		float angle = 181;
		std::string element_name;
		for(int n = 0; attrs[n]; ++n)
		{
			if(!strcmp(attrs[n], "object_name")) element_name = attrs[n + 1];
			else if(!strcmp(attrs[n],"angle")) angle = atof(attrs[n + 1]);
		}
		//not optimal to take ID blind...
		//yafaray_startObjects();
		bool success = yafaray_smoothMesh(yafaray_interface, element_name.c_str(), angle);
		if(!success) yafaray_printWarning(yafaray_interface, ("XMLParser: Couldn't smooth object with object_name='" + element_name + "', angle = " + std::to_string(angle)).c_str());
		//yafaray_endObjects();
		parser.pushState(startElDummy_global, endElDummy_global, "___no_name___");
	}
	else if(!strcmp(element, "instance"))
	{
		std::string element_name;
		for(int n = 0; attrs[n]; n++)
		{
			if(!strcmp(attrs[n], "base_object_name")) element_name = attrs[n + 1];
		}
		parser.pushState(startElInstance_global, endElInstance_global, element_name);
	}
	else yafaray_printWarning(yafaray_interface, ("XMLParser: Skipping unrecognized scene element '" + std::string(element) + "'").c_str());
}

void endElYafaRayXml_global(yafaray_Interface_t *yafaray_interface, XmlParser &parser, const char *element)
{
	if(strcmp(element, "yafaray_xml") != 0) yafaray_printWarning(yafaray_interface, "XMLParser: expected </yafaray_xml> tag!");
	else
	{
		parser.popState();
	}
}

// object-state, i.e. expect only points (vertices), faces and material settings
// since we're supposed to be inside an object block, exit state on "object" element
void startElObject_global(yafaray_Interface_t *yafaray_interface, XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("Object");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);

	if(!strcmp(element, "p"))
	{
		Vec3f p{0.f, 0.f, 0.f};
		Vec3f op{0.f, 0.f, 0.f};
		bool has_orco = false;
		parsePoint_global(yafaray_interface, attrs, p, op, has_orco);
		if(has_orco) yafaray_addVertexWithOrco(yafaray_interface, p.x_, p.y_, p.z_, op.x_, op.y_, op.z_);
		else yafaray_addVertex(yafaray_interface, p.x_, p.y_, p.z_);
	}
	else if(!strcmp(element, "n"))
	{
		Vec3f n(0.0, 0.0, 0.0);
		if(!parseNormal_global(yafaray_interface, attrs, n)) return;
		yafaray_addNormal(yafaray_interface, n.x_, n.y_, n.z_);
	}
	else if(!strcmp(element, "f"))
	{
		std::vector<int> vertices_indices, uv_indices;
		vertices_indices.reserve(3);
		uv_indices.reserve(3);
		for(; attrs && attrs[0]; attrs += 2)
		{
			const std::string attribute = attrs[0];
			if(attribute.size() == 1) switch(attribute[0])
			{
				case 'a' :
				case 'b' :
				case 'c' :
				case 'd' : vertices_indices.push_back(atoi(attrs[1])); break;
				default: yafaray_printWarning(yafaray_interface, ("XMLParser: Ignored wrong attribute '" + attribute + "' in face").c_str());
			}
			else
			{
				if(attribute.substr(0, 3) == "uv_") uv_indices.push_back(atoi(attrs[1]));
			}
		}
		if(uv_indices.empty()) yafaray_addTriangle(yafaray_interface, vertices_indices[0], vertices_indices[1], vertices_indices[2]); //TODO for Quads
		else yafaray_addTriangleWithUv(yafaray_interface, vertices_indices[0], vertices_indices[1], vertices_indices[2], uv_indices[0], uv_indices[1], uv_indices[2]); //TODO for Quads
	}
	else if(!strcmp(element, "uv"))
	{
		float u = 0, v = 0;
		for(; attrs && attrs[0]; attrs += 2)
		{
			switch(attrs[0][0])
			{
				case 'u': u = atof(attrs[1]);
					/*if(!(isValid(u)))
					{
						std::cout << std::scientific << std::setprecision(6) << "XMLParser: invalid value in \"" << element << "\" xml entry: " << attrs[0] << "=" << attrs[1] << ". Replacing with 0.0." << std::endl;
						u = 0.f;
					}*/
					break;
				case 'v': v = atof(attrs[1]);
				/*	if(!(math::isValid(v)))
					{
						std::cout << std::scientific << std::setprecision(6) << "XMLParser: invalid value in \"" << element << "\" xml entry: " << attrs[0] << "=" << attrs[1] << ". Replacing with 0.0." << std::endl;
						v = 0.f;
					}*/
					break;

				default: yafaray_printWarning(yafaray_interface, ("XMLParser: Ignored wrong attribute '" + std::string(attrs[0]) + "' in uv").c_str());
			}
		}
		yafaray_addUv(yafaray_interface, u, v);
	}
	else if(!strcmp(element, "set_material"))
	{
		std::string mat_name(attrs[1]);
		yafaray_setCurrentMaterial(yafaray_interface, mat_name.c_str());
	}
	else if(!strcmp(element, "object_parameters"))
	{
		std::string element_name;
		if(!strcmp(attrs[0], "name")) element_name = attrs[1];
		parser.pushState(startElParammap_global, endElParammap_global, element_name);
	}
}

void endElObject_global(yafaray_Interface_t *yafaray_interface, XmlParser &parser, const char *element)
{
	if(!strcmp(element, "object"))
	{
		//if(!yafaray_endGeometry(yafaray_interface)) std::cout << "XMLParser: Invalid scene state on endGeometry()!" << std::endl; //FIXME?
		parser.popState();
	}
}

void startElInstance_global(yafaray_Interface_t *yafaray_interface, XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("Instance");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);

	if(!strcmp(element, "transform"))
	{
		float m[4][4];
		for(int n = 0; attrs[n]; ++n)
		{
			if(attrs[n][3] == '\0' && attrs[n][0] == 'm' && attrs[n][1] >= '0' && attrs[n][1] <= '3' && attrs[n][2] >= '0' && attrs[n][2] <= '3') //"mij" where i and j are between 0 and 3 (inclusive)
			{
				const int i = attrs[n][1] - '0';
				const int j = attrs[n][2] - '0';
				m[i][j] = atof(attrs[n + 1]);
			}
		}
		yafaray_addInstance(yafaray_interface, parser.stateElementName().c_str(), m);
	}
}

void endElInstance_global(yafaray_Interface_t *yafaray_interface, XmlParser &parser, const char *element)
{
	if(!strcmp(element, "instance"))
	{
		parser.popState();
	}
}
// read a parameter map; take any tag as parameter name
// again, exit when end-element is on of the elements that caused to enter state
// depending on exit element, create appropriate scene element

void startElParammap_global(yafaray_Interface_t *yafaray_interface, XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("Params map");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);
	// support for lists of paramMaps
	if(!strcmp(element, "list_element"))
	{
		yafaray_paramsPushList(yafaray_interface);
		parser.pushState(startElParamlist_global, endElParamlist_global, "___no_name___");
		return;
	}
	parseParam_global(yafaray_interface, attrs, element, parser);
}

void endElParammap_global(yafaray_Interface_t *yafaray_interface, XmlParser &parser, const char *element)
{
	//yafaray_printDebug(yafaray_interface, ("##### endElParammap_global, element='" + std::string(element) + "', element_name='" + std::string(parser.stateElementName()) + "'").c_str());
	const bool exit_state = (parser.currLevel() == parser.stateLevel());
	if(exit_state)
	{
		const std::string element_name = parser.stateElementName();
		if(element_name.empty()) yafaray_printWarning(yafaray_interface, ("XMLParser: No name for scene element '" + std::string(element) + "' available!").c_str());
		else
		{
			if(!strcmp(element, "scene"))
			{
				yafaray_createScene(yafaray_interface);
			}
			else if(!strcmp(element, "material")) yafaray_createMaterial(yafaray_interface, element_name.c_str());
			else if(!strcmp(element, "integrator")) yafaray_createIntegrator(yafaray_interface, element_name.c_str());
			else if(!strcmp(element, "light")) yafaray_createLight(yafaray_interface, element_name.c_str());
			else if(!strcmp(element, "texture"))
			{
				yafaray_createImage(yafaray_interface, element_name.c_str());
				yafaray_paramsSetString(yafaray_interface, "image_name", element_name.c_str());
				yafaray_createTexture(yafaray_interface, element_name.c_str());
			}
			else if(!strcmp(element, "camera")) yafaray_createCamera(yafaray_interface, element_name.c_str());
			else if(!strcmp(element, "background")) yafaray_createBackground(yafaray_interface, element_name.c_str());
			else if(!strcmp(element, "object_parameters")) yafaray_createObject(yafaray_interface, element_name.c_str());
			else if(!strcmp(element, "volumeregion")) yafaray_createVolumeRegion(yafaray_interface, element_name.c_str());
			else if(!strcmp(element, "layers_parameters")) yafaray_setupLayersParameters(yafaray_interface);
			else if(!strcmp(element, "layer")) { yafaray_defineLayer(yafaray_interface); }
			else if(!strcmp(element, "output")) yafaray_createOutput(yafaray_interface, element_name.c_str(), static_cast<yafaray_bool_t>(false));
			else if(!strcmp(element, "render_view")) yafaray_createRenderView(yafaray_interface, element_name.c_str());
			else if(!strcmp(element, "render")) yafaray_setupRender(yafaray_interface);
			else yafaray_printWarning(yafaray_interface, ("XMLParser: Unexpected end-tag of scene element '" + std::string(element) + "'!").c_str());
		}
		parser.popState();
		yafaray_paramsClearAll(yafaray_interface);
	}
}

void startElParamlist_global(yafaray_Interface_t *yafaray_interface, XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("Params list");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);
	parseParam_global(yafaray_interface, attrs, element, parser);
}

void endElParamlist_global(yafaray_Interface_t *yafaray_interface, XmlParser &parser, const char *element)
{
	if(!strcmp(element, "list_element"))
	{
		yafaray_paramsEndList(yafaray_interface);
		parser.popState();
	}
}

END_YAFARAY_XML