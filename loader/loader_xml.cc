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

#include "yafaray_xml_c_api.h"
#include "command_line_parser.h"
#include <csignal>
#include <fstream>

#ifdef WIN32
#include <windows.h>
#endif

yafaray_Logger *yafaray_logger_global = nullptr;
yafaray_RenderControl *yafaray_render_control_global = yafaray_createRenderControl();

#ifdef WIN32
BOOL WINAPI ctrlCHandler_global(DWORD signal)
{
	yafaray_printWarning(yi, "CTRL+C pressed, cancelling.\n");
	if(yafaray_render_control_global)
	{
		yafaray_cancelRendering(yafaray_render_control_global);
		return TRUE;
	}
	else exit(1);
}
#else
void ctrlCHandler_global(int /*signal*/)
{
	if(yafaray_logger_global)
	{
		yafaray_printWarning(yafaray_logger_global, "CTRL+C pressed, cancelling.\n");
		if(yafaray_render_control_global) yafaray_cancelRendering(yafaray_render_control_global);
		else exit(1);
	}
	else exit(1);
}
#endif

bool isNan_global(double value)
{
	return value != value; //To detect NaN when built with fast math option (in that case std::isNan might not work)
}

int main(int argc, char *argv[])
{
	/* handle CTRL+C events */
	#ifdef WIN32
		SetConsoleCtrlHandler(ctrlCHandler_global, TRUE);
	#else
		signal(SIGINT, ctrlCHandler_global);
	#endif

	CliParser parse(argc, argv, 2, 1, "You need to set at least a yafaray's valid XML file.");

	char *version_string = yafaray_xml_getVersionString();
	parse.setAppName("YafaRay XML loader v" + std::string(version_string),
					 std::string{"[OPTIONS]... <input xml file>\n"}
					 + "<input xml file> : A valid yafaray XML file\n"
					 + "*Note: the output file name(s) and parameters are defined in the XML file, in the <output> tags.");

	parse.setOption("v", "version", true, "Displays this program's version.");
	parse.setOption("h", "help", true, "Displays this help text.");
	parse.setOption("vl", "verbosity-level", false, std::string{"Set console verbosity level, options are:\n"}
	+ "                                       \"mute\" (Prints nothing)\n"
	+ "                                       \"error\" (Prints only errors)\n"
	+ "                                       \"warning\" (Prints also warnings)\n"
	+ "                                       \"params\" (Prints also render param messages)\n"
	+ "                                       \"info\" (Prints also basi info messages)\n"
	+ "                                       \"verbose\" (Prints additional info messages)\n"
	+ "                                       \"debug\" (Prints debug messages if any)\n");
	parse.setOption("lvl", "log-verbosity-level", false, "Set log/HTML files verbosity level, options are the same as for the \"verbosity-level\" parameter\n");
	parse.setOption("nodt", "no-date-time", true, "If specified, disables the logging of the date/time in the screen and file logs");
	parse.setOption("ccd", "console-colors-disabled", true, "If specified, disables the Console colors ANSI codes, useful for some 3rd party software that cannot handle ANSI codes well.");
	parse.setOption("ics", "input-color-space", false, std::string{"Sets color space for input color values.\n"}
	+ "                                       This does not affect textures, as they have individual color space parameters in the XML file.\n"
	+ "                                       Available options:\n"
	+ "                                       \"LinearRGB\" (default)\n"
	+ "                                       \"sRGB\"\n"
	+ "                                       \"XYZ\" (experimental)\n");
	parse.setOption("ig", "input-gamma", false, R"(Sets the input gamma for the input color space, 1.0 by default)");
	parse.setOption("sn", "scene-name", false, R"(Scene name from XML file to be rendered. If not specified or does not exist in the XML, the first scene in the XML will be rendered)");
	parse.setOption("in", "integrator-name", false, R"(Surface Integrator name from XML file to be rendered. If not specified or does not exist in the XML, the first surface integrator in the XML will be rendered)");
	parse.setOption("fn", "film-name", false, R"(Film name from XML file to be rendered. If not specified or does not exist in the XML, the first film in the XML will be rendered)");

	const bool parse_ok = parse.parseCommandLine();
	if(!parse_ok)
	{
		parse.printError();
		parse.printUsage();
		return 0;
	}
	else if(parse.isFlagSet("h"))
	{
		parse.printUsage();
		return 0;
	}
	else if(parse.isFlagSet("v"))
	{
		yafaray_printInfo(yafaray_logger_global, ("YafaRay XML loader (LibYafaRay-Xml v" + std::string(version_string) + ")").c_str());
		return 0;
	}

	yafaray_logger_global = yafaray_createLogger("", nullptr, nullptr, YAFARAY_DISPLAY_CONSOLE_NORMAL);

	yafaray_printInfo(yafaray_logger_global, ("YafaRay XML loader (LibYafaRay-Xml v" + std::string(version_string) + ")").c_str());

	const bool no_date_time = parse.isFlagSet("nodt");
	if(no_date_time) yafaray_enablePrintDateTime(yafaray_logger_global, YAFARAY_BOOL_FALSE);

	const bool console_colors_disabled = parse.isFlagSet("ccd");
	if(console_colors_disabled) yafaray_setConsoleLogColorsEnabled(yafaray_logger_global, YAFARAY_BOOL_FALSE);
	else yafaray_setConsoleLogColorsEnabled(yafaray_logger_global, YAFARAY_BOOL_TRUE);

	const std::string verb_level = parse.getOptionString("vl");
	if(verb_level.empty()) yafaray_setConsoleVerbosityLevel(yafaray_logger_global, YAFARAY_LOG_LEVEL_INFO);
	else yafaray_setConsoleVerbosityLevel(yafaray_logger_global, yafaray_logLevelFromString(verb_level.c_str()));

	const std::string log_verb_level = parse.getOptionString("lvl");
	if(log_verb_level.empty()) yafaray_setLogVerbosityLevel(yafaray_logger_global, YAFARAY_LOG_LEVEL_VERBOSE);
	else yafaray_setLogVerbosityLevel(yafaray_logger_global, yafaray_logLevelFromString(verb_level.c_str()));

	std::string input_color_space_string = parse.getOptionString("ics");
	if(input_color_space_string.empty()) input_color_space_string = "LinearRGB";
	float input_gamma = static_cast<float>(parse.getOptionFloat("ig"));
	if(isNan_global(input_gamma)) input_gamma = 1.f;

	const std::vector<std::string> files = parse.getCleanArgs();
	if(files.empty()) return 0;
	const auto &xml_file_path{files.at(0)};

//#define USE_XML_ALTERNATE_MEMORY_PARSING_METHOD
#ifdef USE_XML_ALTERNATE_MEMORY_PARSING_METHOD
	// Test using standard ParseMemory (alternative just to demonstrate memory parsing)
	yafaray_printInfo(yafaray_logger_global, ("Parsing file '" + xml_file_path + "' using alternate ParseMemory method").c_str());
	const std::ifstream xml_stream(xml_file_path);
	std::stringstream xml_stream_buffer;
	xml_stream_buffer << xml_stream.rdbuf();
	const std::string xml_string = xml_stream_buffer.str();
	yafaray_Container *container = yafaray_xml_ParseMemory(yafaray_logger_global, xml_string.c_str(), static_cast<int>(xml_string.size()), input_color_space_string.c_str(), input_gamma);
#else
	// Regular code using standard ParseFile (recommended)
	yafaray_printInfo(yafaray_logger_global, ("Parsing file '" + xml_file_path + "' using standard ParseFile method").c_str());
	yafaray_Container *container = yafaray_xml_ParseFile(yafaray_logger_global, xml_file_path.c_str(), input_color_space_string.c_str(), input_gamma);
#endif

	const std::string scene_name = parse.getOptionString("sn");
	yafaray_Scene *yafaray_scene{nullptr};
	if(scene_name.empty())
	{
		yafaray_scene = yafaray_getSceneFromContainerByIndex(container, 0);
	}
	else if(!yafaray_scene)
	{
		yafaray_printWarning(yafaray_logger_global, ("Scene name '" + scene_name + "' not found in XML file, using the first scene in the file").c_str());
		yafaray_scene = yafaray_getSceneFromContainerByIndex(container, 0);
	}
	else yafaray_scene = yafaray_getSceneFromContainerByName(container, scene_name.c_str());

	const std::string integrator_name = parse.getOptionString("in");
	yafaray_SurfaceIntegrator *yafaray_surface_integrator{nullptr};
	if(integrator_name.empty())
	{
		yafaray_surface_integrator = yafaray_getSurfaceIntegratorFromContainerByIndex(container, 0);
	}
	else if(!yafaray_surface_integrator)
	{
		yafaray_printWarning(yafaray_logger_global, ("Surface Integrator name '" + integrator_name + "' not found in XML file, using the first surface integrator in the file").c_str());
		yafaray_surface_integrator = yafaray_getSurfaceIntegratorFromContainerByIndex(container, 0);
	}
	else yafaray_surface_integrator = yafaray_getSurfaceIntegratorFromContainerByName(container, integrator_name.c_str());

	const std::string film_name = parse.getOptionString("fn");
	yafaray_Film *yafaray_film{nullptr};
	if(film_name.empty())
	{
		yafaray_film = yafaray_getFilmFromContainerByIndex(container, 0);
	}
	else if(!yafaray_film)
	{
		yafaray_printWarning(yafaray_logger_global, ("Film name '" + film_name + "' not found in XML file, using the first film in the file").c_str());
		yafaray_film = yafaray_getFilmFromContainerByIndex(container, 0);
	}
	else yafaray_film = yafaray_getFilmFromContainerByName(container, film_name.c_str());

	yafaray_setRenderControlForNormalStart(yafaray_render_control_global);
	yafaray_SceneModifiedFlags yafaray_scene_modified_flags{YAFARAY_SCENE_MODIFIED_NOTHING};
	yafaray_scene_modified_flags = yafaray_checkAndClearSceneModifiedFlags(yafaray_scene);
	yafaray_preprocessScene(yafaray_scene, yafaray_render_control_global, yafaray_scene_modified_flags);
	yafaray_RenderMonitor *yafaray_render_monitor = yafaray_createRenderMonitor(nullptr, nullptr, YAFARAY_DISPLAY_CONSOLE_NORMAL);
	yafaray_preprocessSurfaceIntegrator(yafaray_render_monitor, yafaray_surface_integrator, yafaray_render_control_global, yafaray_scene);
	yafaray_render(yafaray_render_control_global, yafaray_render_monitor, yafaray_surface_integrator, yafaray_film);
	yafaray_destroyRenderMonitor(yafaray_render_monitor);
	yafaray_destroyRenderControl(yafaray_render_control_global);
	yafaray_destroyContainerAndContainedPointers(container);
	yafaray_destroyLogger(yafaray_logger_global);
	yafaray_xml_destroyCharString(version_string);
	return 0;
}
