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
#include <libxml/parser.h>
#include "common/version_build_info.h"
#include "common/element_parser_utils.h"
#include <sstream>
#include <iostream>

//#define DEBUG_XML

namespace yafaray_xml
{

void startElement(void *user_data, const xmlChar *name, const xmlChar **attrs)
{
#ifdef DEBUG_XML
	std::cout << "startElement <" << name << getElementAttrs((const char **)attrs) << ">" << std::endl;

#endif //DEBUG_XML
	XmlParser &parser = *static_cast<XmlParser *>(user_data);
	parser.startElement((const char *)name, (const char **)attrs);
}

void endElement(void *user_data, const xmlChar *name)
{
#ifdef DEBUG_XML
	std::cout << "endElement </" << name << ">" << std::endl;
#endif //DEBUG_XML
	XmlParser &parser = *static_cast<XmlParser *>(user_data);
	parser.endElement((const char *)name);
}

enum XmlErrorSeverity { Warning, Error, FatalError };
static void xmlErrorProcessing(XmlErrorSeverity xml_error_severity, void *user_data)
{
	XmlParser &parser = *static_cast<XmlParser *>(user_data);
	std::stringstream message_stream;
	message_stream << "XMLParser ";
	switch(xml_error_severity)
	{
		case XmlErrorSeverity::FatalError: message_stream << "fatal error: "; break;
		case XmlErrorSeverity::Error: message_stream << "error: "; break;
		case XmlErrorSeverity::Warning:
		default: message_stream << "warning: "; break;
	}

	xmlErrorPtr error = xmlGetLastError();
	message_stream << "(error code " << error->code << ") [line:" << error->line << ", col:" << error->int2 << "] " << error->message;
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
	xmlErrorProcessing(XmlErrorSeverity::Warning, user_data);
}

static void myError(void *user_data, const char *msg, ...)
{
	xmlErrorProcessing(XmlErrorSeverity::Error, user_data);
}

static void myFatalError(void *user_data, const char *msg, ...)
{
	xmlErrorProcessing(XmlErrorSeverity::FatalError, user_data);
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


XmlParser::XmlParser(yafaray_Logger *yafaray_logger, const char *input_color_space, float input_gamma) :
		yafaray_logger_{yafaray_logger},
		yafaray_param_map_{yafaray_createParamMap()},
		yafaray_param_map_list_{yafaray_createParamMapList()}
{
	if(yafaray_param_map_) yafaray_setInputColorSpace(yafaray_param_map_, input_color_space, input_gamma);
	std::setlocale(LC_NUMERIC, "C"); //To make sure floating points in the xml file are evaluated using the dot and not a comma in some locales
	pushState(startElDocument, endElDocument, "root", nullptr);
}

XmlParser::~XmlParser()
{
	yafaray_destroyParamMapList(yafaray_param_map_list_);
	yafaray_destroyParamMap(yafaray_param_map_);
}

void XmlParser::pushState(StartElementCb_t start, EndElementCb_t end, const char *element, const char **element_attrs)
{
	ParserState state;
	state.start_ = start;
	state.end_ = end;
	state.element_ = element;
	state.element_name_ = getElementName(*this, element_attrs);
	state.element_attributes_ = getElementAttrs(element_attrs);
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
	yafaray_addSceneToContainer(yafaray_container_, yafaray_scene_);
}

void XmlParser::createSurfaceIntegrator(const char *name)
{
	yafaray_surface_integrator_ = yafaray_createSurfaceIntegrator(yafaray_logger_, name, yafaray_param_map_);
	yafaray_addSurfaceIntegratorToContainer(yafaray_container_, yafaray_surface_integrator_);
}

void XmlParser::createFilm(const char *name)
{
	yafaray_film_ = yafaray_createFilm(yafaray_logger_, yafaray_surface_integrator_, name, yafaray_param_map_);
	yafaray_addFilmToContainer(yafaray_container_, yafaray_film_);
}

std::tuple<bool, yafaray_Container *> XmlParser::parseXmlFile(yafaray_Logger *yafaray_logger, const char *xml_file_path, const char *input_color_space, float input_gamma) noexcept
{
	XmlParser parser{yafaray_logger, input_color_space, input_gamma};
	if(!xml_file_path || xmlSAXUserParseFile(&my_handler_global, &parser, xml_file_path) < 0)
	{
		yafaray_printError(parser.getLogger(), ("XMLParser: Error parsing the file " + std::string(xml_file_path)).c_str());
		return {};
	}
	else return {true, parser.getContainer()};
}

std::tuple<bool, yafaray_Container *> XmlParser::parseXmlMemory(yafaray_Logger *yafaray_logger, const char *xml_buffer, int xml_buffer_size, const char *input_color_space, float input_gamma) noexcept
{
	XmlParser parser{yafaray_logger, input_color_space, input_gamma};
	if(!xml_buffer || xml_buffer_size <= 0 || xmlSAXUserParseMemory(&my_handler_global, &parser, xml_buffer, xml_buffer_size) < 0)
	{
		yafaray_printError(parser.getLogger(), "XMLParser: Error parsing a memory buffer");
		return {};
	}
	else return {true, parser.getContainer()};
}

std::string XmlParser::printStateStack() const
{
	std::stringstream ss;
	ss << "XML element stack up to the error:" << std::endl;
	for(const auto &state : state_stack_)
	{
		ss << state.print() << std::endl;
	}
	return ss.str();
}

std::string ParserState::print() const
{
	std::stringstream ss;
	ss << "Level:" << level_ << " Element:'" << element_ << "' name:'" << element_name_ << "' attrs:" << element_attributes_;
	return ss.str();
}
} //namespace yafaray_xml