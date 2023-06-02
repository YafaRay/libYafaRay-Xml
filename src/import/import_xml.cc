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

namespace yafaray_xml
{

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

void startElement(void *user_data, const xmlChar *name, const xmlChar **attrs)
{
	XmlParser &parser = *static_cast<XmlParser *>(user_data);
	parser.startElement((const char *)name, (const char **)attrs);
}

void endElement(void *user_data, const xmlChar *name)
{
	XmlParser &parser = *static_cast<XmlParser *>(user_data);
	parser.endElement((const char *)name);
}

enum XmlErrorSeverity { Warning, Error, FatalError };
static void xmlErrorProcessing(XmlErrorSeverity xml_error_severity, void *user_data, const char *msg, ...)
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
	switch(xml_error_severity)
	{
		case XmlErrorSeverity::FatalError:
		case XmlErrorSeverity::Error: yafaray_printError(parser.getLogger(), message_stream.str().c_str()); break;
		case XmlErrorSeverity::Warning:
		default: yafaray_printWarning(parser.getLogger(), message_stream.str().c_str()); break;
	}
}

static void myWarning(void *user_data, const char *msg, ...)
{
	va_list args;
	xmlErrorProcessing(XmlErrorSeverity::Warning, user_data, msg, args);
}

static void myError(void *user_data, const char *msg, ...)
{
	va_list args;
	xmlErrorProcessing(XmlErrorSeverity::Error, user_data, msg, args);
}

static void myFatalError(void *user_data, const char *msg, ...)
{
	va_list args;
	xmlErrorProcessing(XmlErrorSeverity::FatalError, user_data, msg, args);
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
	nullptr, //  startDocumentSAXFunc startDocument;
	nullptr, //  endDocumentSAXFunc endDocument;
	startElement, //  startElementSAXFunc startElement;
	endElement, //  endElementSAXFunc endElement;
	nullptr,
	nullptr, //  charactersSAXFunc characters;
	nullptr,
	nullptr,
	nullptr,
	myWarning,
	myError,
	myFatalError
};

std::tuple<bool, yafaray_Scene *, yafaray_SurfaceIntegrator *, yafaray_Film *> XmlParser::parseXmlFile(yafaray_Logger *yafaray_logger, const char *xml_file_path, const char *input_color_space, float input_gamma) noexcept
{
	XmlParser parser{yafaray_logger, input_color_space, input_gamma};
	if(!xml_file_path || xmlSAXUserParseFile(&my_handler_global, &parser, xml_file_path) < 0)
	{
		yafaray_printError(parser.getLogger(), ("XMLParser: Error parsing the file " + std::string(xml_file_path)).c_str());
		return {};
	}
	else return {true, parser.getScene(), parser.getSurfaceIntegrator(), parser.getFilm()};
}

std::tuple<bool, yafaray_Scene *, yafaray_SurfaceIntegrator *, yafaray_Film *> XmlParser::parseXmlMemory(yafaray_Logger *yafaray_logger, const char *xml_buffer, int xml_buffer_size, const char *input_color_space, float input_gamma) noexcept
{
	XmlParser parser{yafaray_logger, input_color_space, input_gamma};
	if(!xml_buffer || xml_buffer_size <= 0 || xmlSAXUserParseMemory(&my_handler_global, &parser, xml_buffer, xml_buffer_size) < 0)
	{
		yafaray_printError(parser.getLogger(), "XMLParser: Error parsing a memory buffer");
		return {};
	}
	else return {true, parser.getScene(), parser.getSurfaceIntegrator(), parser.getFilm()};
}

/*=============================================================
/ parser functions
=============================================================*/

XmlParser::XmlParser(yafaray_Logger *yafaray_logger, const char *input_color_space, float input_gamma) :
	yafaray_logger_{yafaray_logger},
	yafaray_param_map_{yafaray_createParamMap()},
	yafaray_param_map_list_{yafaray_createParamMapList()}
{
	if(yafaray_param_map_) yafaray_setInputColorSpace(yafaray_param_map_, input_color_space, input_gamma);
	std::setlocale(LC_NUMERIC, "C"); //To make sure floating points in the xml file are evaluated using the dot and not a comma in some locales
	pushState(startElDocument, endElDocument, "");
}

XmlParser::~XmlParser()
{
	yafaray_destroyParamMapList(yafaray_param_map_list_);
	yafaray_destroyParamMap(yafaray_param_map_);
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

void XmlParser::createScene(const char *name)
{
	yafaray_scene_ = yafaray_createScene(yafaray_logger_, name);
}

void XmlParser::createSurfaceIntegrator(const char *name)
{
	yafaray_surface_integrator_ = yafaray_createSurfaceIntegrator(yafaray_logger_, name, yafaray_param_map_);
}

void XmlParser::createFilm(const char *name)
{
	yafaray_film_ = yafaray_createFilm(yafaray_logger_, yafaray_surface_integrator_, name, yafaray_param_map_);
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
			case 't' : time_step = atoi(attrs[1]); break;
			default: yafaray_printWarning(yafaray_logger, ("XMLParser: Ignored wrong attribute " + std::string(attrs[0]) + " in point").c_str());
		}
	}
}

static bool parseNormal(yafaray_Logger *yafaray_logger, const char **attrs, Vec3f &n, int &time_step)
{
	int compo_read = 0;
	for(; attrs && attrs[0]; attrs += 2)
	{
		if(attrs[0][1] != 0)
		{
			yafaray_printWarning(yafaray_logger, ("XMLParser: Ignored wrong attribute " + std::string(attrs[0]) + " in normal").c_str());
			continue; //it is not a single character
		}
		switch(attrs[0][0])
		{
			case 'x' : n.x_ = static_cast<float>(atof(attrs[1])); compo_read++; break;
			case 'y' : n.y_ = static_cast<float>(atof(attrs[1])); compo_read++; break;
			case 'z' : n.z_ = static_cast<float>(atof(attrs[1])); compo_read++; break;
			case 't' : time_step = atoi(attrs[1]); compo_read++; break;
			default: yafaray_printWarning(yafaray_logger, ("XMLParser: Ignored wrong attribute " + std::string(attrs[0]) + " in normal").c_str());
		}
	}
	return (compo_read == 3);
}

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

/*=============================================================
/ start- and endElement callbacks for the different states
=============================================================*/

void endElDummy(XmlParser &parser, const char *)
{
	parser.popState();
}

void startElDummy(XmlParser &parser, const char *, const char **)
{
	parser.pushState(startElDummy, endElDummy, "___no_name___");
}

class Version final
{
	public:
		explicit Version(const std::string &version_string);
		[[nodiscard]] std::string getString() const { return std::to_string(major_) + "." + std::to_string(minor_) + "." + std::to_string(patch_); }
		[[nodiscard]] int getMajor() const { return major_; }
		[[nodiscard]] int getMinor() const { return minor_; }
		[[nodiscard]] int getPatch() const { return patch_; }
		[[nodiscard]] bool isValid() const { return major_ == 0 && minor_ == 0 && patch_ == 0; }

	private:
		int major_ = 0;
		int minor_ = 0;
		int patch_ = 0;
};

Version::Version(const std::string &version_string)
{
	bool error = false;
	size_t pos_first_dot = version_string.find('.');
	if(pos_first_dot < version_string.size())
	{
		major_ = std::stoi(version_string.substr(0, pos_first_dot));
		std::string remaining_string = version_string.substr(pos_first_dot + 1);
		size_t pos_second_dot = remaining_string.find('.');
		if(pos_second_dot < remaining_string.size())
		{
			minor_ = std::stoi(remaining_string.substr(0, pos_second_dot));
			patch_ = std::stoi(remaining_string.substr(pos_second_dot + 1));
		}
		else error = true;
	}
	else error = true;

	if(error)
	{
		major_ = 0;
		minor_ = 0;
		patch_ = 0;
	}
}

void startElDocument(XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("Document");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);

	if(strcmp(element, "yafaray_xml") == 0)
	{
		if(!attrs || !attrs[0])
		{
			yafaray_printError(parser.getLogger(), "XMLParser: No attributes for yafaray_xml element, cannot check xml format version");
		}
		else if(!strcmp(attrs[0], "format_version"))
		{
			if(attrs[1])
			{
				const std::string format_version_string = attrs[1];
				Version xml_version(format_version_string);
				const int lib_version_major = build_info::getVersionMajor();
				const int lib_version_minor = build_info::getVersionMinor();
				const int lib_version_patch = build_info::getVersionPatch();
				yafaray_printVerbose(parser.getLogger(), ("XMLParser: The XML format version is: '" + xml_version.getString() + "'").c_str());
				if(xml_version.isValid())
				{
					yafaray_printError(parser.getLogger(), ("XMLParser: The XML format version '" + format_version_string + "' is malformed and cannot be checked.").c_str());
				}
				else if((xml_version.getMajor() > lib_version_major) ||
						(xml_version.getMajor() == lib_version_major && xml_version.getMinor() > lib_version_minor) ||
						(xml_version.getMajor() == lib_version_major && xml_version.getMinor() == lib_version_minor && xml_version.getPatch() > lib_version_patch))
				{
					yafaray_printError(parser.getLogger(), ("XMLParser: The XML format version '" + format_version_string + "' is higher than the libYafaRay-Xml version '" + build_info::getVersionString() + "'").c_str());
				}
			}
			else yafaray_printError(parser.getLogger(), "XMLParser: No format version specified for format_version attribute in yafaray_xml element, cannot check xml format version");
		}
		else
		{
			yafaray_printWarning(parser.getLogger(), "XMLParser: Attribute for yafaray_xml element does not match 'format_version'!");
			return;
		}
		parser.pushState(startElYafaRayXml, endElYafaRayXml, "___no_name___");
	}
	else yafaray_printWarning(parser.getLogger(), ("XMLParser: unexpected element <" + std::string(element) + ">, where the element 'yafaray_xml' was expected, skipping...").c_str());
}

void endElDocument(XmlParser &parser, const char *)
{
	yafaray_printVerbose(parser.getLogger(), "XMLParser: Finished document");
}

// scene-state, i.e. expect only primary elements
// such as light, material, texture, object, integrator, render...

void startElYafaRayXml(XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("YafarayXml");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);

	if(!strcmp(element, "scene"))
	{
		parser.pushState(startElScene, endElScene, "___no_name___");
	}
	else if(!strcmp(element, "surface_integrator"))
	{
		parser.pushState(startElSurfaceIntegrator, endElSurfaceIntegrator, "___no_name___");
	}
	else if(!strcmp(element, "film"))
	{
		parser.pushState(startElFilm, endElFilm, "___no_name___");
	}
	else yafaray_printWarning(parser.getLogger(), ("XMLParser: Skipping unrecognized YafaRayXml element '" + std::string(element) + "'").c_str());
}

void endElYafaRayXml(XmlParser &parser, const char *element)
{
	if(strcmp(element, "yafaray_xml") == 0)
	{
		parser.popState();
	}
}

void startElScene(XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("Scene");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);

	if(!strcmp(element, "scene_parameters") || !strcmp(element, "material") || !strcmp(element, "light") || !strcmp(element, "texture") || !strcmp(element, "volumeregion") || !strcmp(element, "image") || !strcmp(element, "light"))
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
	else if(!strcmp(element, "background"))
	{
		parser.pushState(startElParammap, endElParammap, "___no_name___");
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
		parser.pushState(startElDummy, endElDummy, "___no_name___");
	}
	else if(!strcmp(element, "accelerator"))
	{
		parser.pushState(startElParammap, endElParammap, "___no_name___");
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

void startElFilm(XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("Film");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);

	if(!strcmp(element, "film_parameters") || !strcmp(element, "camera") || !strcmp(element, "output"))
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
	else if(!strcmp(element, "layer"))
	{
		parser.pushState(startElParammap, endElParammap, "___no_name___");
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

// object-state, i.e. expect only points (vertices), faces and material settings
// since we're supposed to be inside an object block, exit state on "object" element
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
		std::string mat_name(attrs[1]);
		size_t material_id;
		yafaray_getMaterialId(parser.getScene(), &material_id, mat_name.c_str());
		parser.setMaterialIdCurrent(material_id);
	}
	else if(!strcmp(element, "object_parameters"))
	{
		std::string element_name;
		if(!strcmp(attrs[0], "name")) element_name = attrs[1];
		parser.pushState(startElParammap, endElParammap, element_name);
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

// read a parameter map; take any tag as parameter name
// again, exit when end-element is on of the elements that caused to enter state
// depending on exit element, create appropriate element

void startElParammap(XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("Params map");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);
	if(!strcmp(element, "shader_node"))
	{
		parser.pushState(startElShaderNode, endElShaderNode, "___no_name___");
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
		if(element_name.empty() && strcmp(element, "createInstance") != 0 && strcmp(element, "addInstanceObject") != 0 && strcmp(element, "addInstanceOfInstance") != 0 && strcmp(element, "addInstanceMatrix") != 0 && strcmp(element, "background") != 0 && strcmp(element, "surface_integrator") != 0 && strcmp(element, "volume_integrator") != 0)
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

void startElShaderNode(XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("Params list");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);
	parseParam(parser.getParamMap(), attrs, element);
}

void endElShaderNode(XmlParser &parser, const char *element)
{
	if(!strcmp(element, "shader_node"))
	{
		parser.addParamMapToList();
		parser.clearParamMap();
		parser.popState();
	}
}

void endElCreateInstance(XmlParser &parser, const char *element)
{
	if(!strcmp(element, "createInstance"))
	{
		parser.popState();
	}
}

void endElAddInstanceObject(XmlParser &parser, const char *element)
{
	if(!strcmp(element, "addInstanceObject"))
	{
		parser.popState();
	}
}

void endElAddInstanceOfInstance(XmlParser &parser, const char *element)
{
	if(!strcmp(element, "addInstanceOfInstance"))
	{
		parser.popState();
	}
}

void startElAddInstanceMatrix(XmlParser &parser, const char *element, const char **attrs)
{
	parser.setLastSection("AddInstanceMatrix");
	parser.setLastElementName(element);
	parser.setLastElementNameAttrs(attrs);

	if(!strcmp(element, "transform"))
	{
		double m[4 * 4];
		for(int n = 0; attrs[n]; ++n)
		{
			if(attrs[n][3] == '\0' && attrs[n][0] == 'm' && attrs[n][1] >= '0' && attrs[n][1] <= '3' && attrs[n][2] >= '0' && attrs[n][2] <= '3') //"mij" where i and j are between 0 and 3 (inclusive)
			{
				const int i = attrs[n][1] - '0';
				const int j = attrs[n][2] - '0';
				m[4 * i + j] = atof(attrs[n + 1]);
			}
		}
		yafaray_addInstanceMatrixArray(parser.getScene(), parser.getInstanceIdCurrent(), m, parser.getTimeCurrent());
	}
}

void endElAddInstanceMatrix(XmlParser &parser, const char *element)
{
	if(!strcmp(element, "addInstanceMatrix"))
	{
		parser.popState();
	}
}

} //namespace yafaray_xml