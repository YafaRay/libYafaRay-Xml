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

#ifndef LIBYAFARAY_XML_IMPORT_XML_H
#define LIBYAFARAY_XML_IMPORT_XML_H

#include <yafaray_c_api.h>
#include <list>
#include <vector>
#include <string>
#include <memory>

namespace yafaray_xml
{

class XmlParser;
enum ColorSpace : int;

typedef void (*StartElementCb_t)(XmlParser &p, const char *element, const char **attrs);
typedef void (*EndElementCb_t)(XmlParser &p, const char *element);

struct ParserState
{
	[[nodiscard]] std::string print() const;
	StartElementCb_t start_;
	EndElementCb_t end_;
	std::string element_;
	std::string element_name_;
	std::string element_attributes_;
	int level_;
};

class XmlParser final
{
	public:
		XmlParser(yafaray_Logger *yafaray_logger, const char *input_color_space, float input_gamma);
		~XmlParser();
		void pushState(StartElementCb_t start, EndElementCb_t end, const char *element, const char **element_attrs);
		void popState();
		[[nodiscard]] std::string printStateStack() const;
		void startElement(const char *element, const char **attrs) { ++level_; if(current_) current_->start_(*this, element, attrs); }
		void endElement(const char *element) { if(current_) current_->end_(*this, element); --level_; }
		[[nodiscard]] std::string stateElementName() const { return current_->element_name_; }
		[[nodiscard]] int currLevel() const { return level_; }
		[[nodiscard]] int stateLevel() const { return current_ ? current_->level_ : -1; }
		[[nodiscard]] yafaray_Logger *getLogger() { return yafaray_logger_; }
		void createScene(const char *name);
		[[nodiscard]] yafaray_Scene *getScene() { return yafaray_scene_; }
		void createSurfaceIntegrator(const char *name);
		[[nodiscard]] yafaray_SurfaceIntegrator *getSurfaceIntegrator() { return yafaray_surface_integrator_; }
		void createFilm(const char *name);
		[[nodiscard]] yafaray_Film *getFilm() { return yafaray_film_; }
		[[nodiscard]] yafaray_ParamMap *getParamMap() { return yafaray_param_map_; }
		void clearParamMap() { yafaray_clearParamMap(yafaray_param_map_); }
		void clearParamMapList() { yafaray_clearParamMapList(yafaray_param_map_list_); }
		[[nodiscard]] yafaray_ParamMapList *getParamMapList() { return yafaray_param_map_list_; }
		void addParamMapToList() { yafaray_addParamMapToList(yafaray_param_map_list_, yafaray_param_map_); }
		[[nodiscard]] size_t getInstanceIdCurrent() const { return instance_id_current_; }
		void setInstanceIdCurrent(size_t instance_id_current) { instance_id_current_ = instance_id_current; }
		[[nodiscard]] size_t getObjectIdCurrent() const { return object_id_current_; }
		void setObjectIdCurrent(size_t object_id_current) { object_id_current_ = object_id_current; }
		[[nodiscard]] size_t getMaterialIdCurrent() const { return material_id_current_; }
		void setMaterialIdCurrent(size_t material_id_current) { material_id_current_ = material_id_current; }
		[[nodiscard]] float getTimeCurrent() const { return time_current_; }
		void setTimeCurrent(float time_current) { time_current_ = time_current; }
		[[nodiscard]] static std::tuple<bool, yafaray_Scene *, yafaray_SurfaceIntegrator *, yafaray_Film *> parseXmlFile(yafaray_Logger *yafaray_logger, const char *xml_file_path, const char *input_color_space, float input_gamma) noexcept;
		[[nodiscard]] static std::tuple<bool, yafaray_Scene *, yafaray_SurfaceIntegrator *, yafaray_Film *> parseXmlMemory(yafaray_Logger *yafaray_logger, const char *xml_buffer, int xml_buffer_size, const char *input_color_space, float input_gamma) noexcept;

	private:
		std::vector<ParserState> state_stack_;
		ParserState *current_ = nullptr;
		int level_ = 0;
		yafaray_Logger *yafaray_logger_ = nullptr;
		yafaray_Scene *yafaray_scene_ = nullptr;
		yafaray_SurfaceIntegrator *yafaray_surface_integrator_ = nullptr;
		yafaray_Film *yafaray_film_ = nullptr;
		yafaray_ParamMap *yafaray_param_map_ = nullptr;
		yafaray_ParamMapList *yafaray_param_map_list_ = nullptr;
		size_t instance_id_current_ = 0;
		size_t object_id_current_ = 0;
		size_t material_id_current_ = 0;
		float time_current_ = 0.f;
};

void parseParam(yafaray_ParamMap *yafaray_param_map, const char **attrs, const char *param_name);

// state callbacks:
void startElDocument(XmlParser &p, const char *element, const char **attrs);
void endElDocument(XmlParser &p, const char *element);
void startElYafaRayContainer(XmlParser &p, const char *element, const char **attrs);
void endElYafaRayContainer(XmlParser &p, const char *element);
void startElScene(XmlParser &p, const char *element, const char **attrs);
void endElScene(XmlParser &p, const char *element);
void startElSurfaceIntegrator(XmlParser &p, const char *element, const char **attrs);
void endElSurfaceIntegrator(XmlParser &p, const char *element);
void startElFilm(XmlParser &p, const char *element, const char **attrs);
void endElFilm(XmlParser &p, const char *element);
void startElObject(XmlParser &p, const char *element, const char **attrs);
void endElObject(XmlParser &p, const char *element);
void startElParammap(XmlParser &p, const char *element, const char **attrs);
void endElParammap(XmlParser &parser, const char *element);
void startElShaderNode(XmlParser &p, const char *element, const char **attrs);
void endElShaderNode(XmlParser &p, const char *element);
void startElAddInstanceObject(XmlParser &p, const char *element, const char **attrs);
void endElAddInstanceObject(XmlParser &p, const char *element);
void startElCreateInstance(XmlParser &p, const char *element, const char **attrs);
void endElCreateInstance(XmlParser &p, const char *element);
void startElAddInstanceOfInstance(XmlParser &p, const char *element, const char **attrs);
void endElAddInstanceOfInstance(XmlParser &p, const char *element);
void startElAddInstanceMatrix(XmlParser &p, const char *element, const char **attrs);
void endElAddInstanceMatrix(XmlParser &p, const char *element);
void startElInstanceMatrixTransform(XmlParser &p, const char *element, const char **attrs);
void endElInstanceMatrixTransform(XmlParser &p, const char *element);
void startElDummy(XmlParser &p, const char *element, const char **attrs);
void endElDummy(XmlParser &p, const char *element);

} //namespace yafaray_xml

#endif // LIBYAFARAY_XML_IMPORT_XML_H
