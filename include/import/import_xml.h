#pragma once
/****************************************************************************
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
 */

#ifndef YAFARAY_IMPORT_XML_H
#define YAFARAY_IMPORT_XML_H

#include "common/yafaray_xml_common.h"
#include <yafaray_c_api.h>
#include <list>
#include <vector>
#include <string>
#include <memory>

BEGIN_YAFARAY_XML

class XmlParser;
enum ColorSpace : int;

typedef void (*StartElementCb_t)(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element, const char **attrs);
typedef void (*EndElementCb_t)(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element);

struct ParserState
{
	StartElementCb_t start_;
	EndElementCb_t end_;
	std::string element_name_;
	int level_;
	std::string last_section_; //! to show last section previous to a XML Parser error
	std::string last_element_; //! to show last element previous to a XML Parser error
	std::string last_element_attrs_; //! to show last element attributes previous to a XML Parser error
};

struct Vec3f
{
	explicit Vec3f() = default;
	explicit Vec3f(float x, float y, float z) : x_(x), y_(y), z_(z) { }
	float x_, y_, z_;
};

struct Rgba
{
	explicit Rgba(float v) : Rgba(v, 1.f) { }
	explicit Rgba(float v, float a) : Rgba(v, v, v, a) { }
	explicit Rgba(float r, float g, float b, float a) : r_(r), g_(g), b_(b), a_(a) { }
	float r_, g_, b_, a_;
};

class XmlParser final
{
	public:
		XmlParser(yafaray_Interface_t *yafaray_interface);
		void pushState(StartElementCb_t start, EndElementCb_t end, const std::string &element_name);
		void popState();
		void startElement(yafaray_Interface_t *yafaray_interface, const char *element, const char **attrs) { ++level_; if(current_) current_->start_(yafaray_interface, *this, element, attrs); }
		void endElement(yafaray_Interface_t *yafaray_interface, const char *element) { if(current_) current_->end_(yafaray_interface, *this, element); --level_; }
		std::string stateElementName() { return current_->element_name_; }
		int currLevel() const { return level_; }
		int stateLevel() const { return current_ ? current_->level_ : -1; }
		void setLastSection(const std::string &section) { current_->last_section_ = section; }
		void setLastElementName(const char *element_name);
		void setLastElementNameAttrs(const char **element_attrs);
		std::string getLastSection() const { return current_->last_section_; }
		std::string getLastElementName() const { return current_->last_element_; }
		std::string getLastElementNameAttrs() const { return current_->last_element_attrs_; }
		yafaray_Interface_t *getInterface() { return yafaray_interface_; }
		int getInstanceIdCurrent() const { return instance_id_current_; }
		void setInstanceIdCurrent(int instance_id_current) { instance_id_current_ = instance_id_current; }
		float getTimeCurrent() const { return time_current_; }
		void setTimeCurrent(float time_current) { time_current_ = time_current; }
		float *matrixCurrent() { return reinterpret_cast<float *>(matrix_current_); }
		static bool parseXmlFile(yafaray_Interface_t *yafaray_interface, const char *xml_file_path) noexcept;
		static bool parseXmlMemory(yafaray_Interface_t *yafaray_interface, const char *xml_buffer, unsigned int xml_buffer_size) noexcept;

	private:
		std::vector<ParserState> state_stack_;
		ParserState *current_ = nullptr;
		int level_ = 0;
		yafaray_Interface_t *yafaray_interface_ = nullptr;
		float matrix_current_[4][4];
		int instance_id_current_ = -1;
		float time_current_ = 0.f;
};

// state callbacks:
void startElDocument_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element, const char **attrs);
void endElDocument_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element);
void startElYafaRayXml_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element, const char **attrs);
void endElYafaRayXml_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element);
void startElObject_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element, const char **attrs);
void endElObject_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element);
void startElParammap_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element, const char **attrs);
void endElParammap_global(yafaray_Interface_t *yafaray_interface, XmlParser &parser, const char *element);
void startElParamlist_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element, const char **attrs);
void endElParamlist_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element);
void startElAddInstanceObject_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element, const char **attrs);
void endElAddInstanceObject_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element);
void startElCreateInstance_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element, const char **attrs);
void endElCreateInstance_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element);
void startElAddInstanceOfInstance_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element, const char **attrs);
void endElAddInstanceOfInstance_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element);
void startElAddInstanceMatrix_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element, const char **attrs);
void endElAddInstanceMatrix_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element);
void startElInstanceMatrixTransform_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element, const char **attrs);
void endElInstanceMatrixTransform_global(yafaray_Interface_t *yafaray_interface, XmlParser &p, const char *element);

END_YAFARAY_XML

#endif // YAFARAY_IMPORT_XML_H
