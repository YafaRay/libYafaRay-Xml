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

#ifdef WIN32
#include <windows.h>
#endif

yafaray_Interface_t *yi = nullptr;

#ifdef WIN32
BOOL WINAPI ctrlCHandler_global(DWORD signal)
{
	yafaray_printWarning(yi, "CTRL+C pressed, cancelling.\n");
	if(yi)
	{
		yafaray_cancelRendering(yi);
		return TRUE;
	}
	else exit(1);
}
#else
void ctrlCHandler_global(int /*signal*/)
{
	yafaray_printWarning(yi, "CTRL+C pressed, cancelling.\n");
	if(yi) yafaray_cancelRendering(yi);
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

	const int version_string_size = 100;
	char *version_c_string = (char *) malloc(version_string_size * sizeof(char));
	yafaray_xml_getVersionString(version_c_string, version_string_size);
	const std::string version_string {version_c_string};
	free(version_c_string);
	parse.setAppName("YafaRay XML loader v" + version_string,
					 "[OPTIONS]... <input xml file>\n<input xml file> : A valid yafaray XML file\n*Note: the output file name(s) and parameters are defined in the XML file, in the <output> tags.");

	parse.setOption("v", "version", true, "Displays this program's version.");
	parse.setOption("h", "help", true, "Displays this help text.");
	parse.setOption("l", "log-file-output", false, R"(Enable log file output(s): "none", "txt", "html" or "txt+html". Log file name will be same as selected image name,)");
	parse.setOption("vl", "verbosity-level", false, "Set console verbosity level, options are:\n                                       \"mute\" (Prints nothing)\n                                       \"error\" (Prints only errors)\n                                       \"warning\" (Prints also warnings)\n                                       \"params\" (Prints also render param messages)\n                                       \"info\" (Prints also basi info messages)\n                                       \"verbose\" (Prints additional info messages)\n                                       \"debug\" (Prints debug messages if any)\n");
	parse.setOption("lvl", "log-verbosity-level", false, "Set log/HTML files verbosity level, options are the same as for the \"verbosity-level\" parameter\n");
	parse.setOption("nodt", "no-date-time", true, "If specified, disables the logging of the date/time in the screen and file logs");
	parse.setOption("ccd", "console-colors-disabled", true, "If specified, disables the Console colors ANSI codes, useful for some 3rd party software that cannot handle ANSI codes well.");
	parse.setOption("ics", "input-color-space", false, "Sets color space for input color values.\n                                       This does not affect textures, as they have individual color space parameters in the XML file.\n                                       Available options:\n                                       LinearRGB (default)\n                                       sRGB\n                                       XYZ (experimental)\n");
	parse.setOption("ig", "input-gamma", false, R"(Sets the input gamma for the input color space, 1.0 by default)");

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
		yafaray_printInfo(yi, ("YafaRay XML loader v" + version_string).c_str());
		return 0;
	}

	yi = yafaray_createInterface(YAFARAY_INTERFACE_FOR_RENDERING, nullptr, nullptr, nullptr, YAFARAY_DISPLAY_CONSOLE_NORMAL);

	yafaray_printInfo(yi, ("YafaRay XML loader (LibYafaRay-Xml v" + version_string + ")").c_str());

	const bool no_date_time = parse.isFlagSet("nodt");
	if(no_date_time) yafaray_enablePrintDateTime(yi, YAFARAY_BOOL_FALSE);

	const bool console_colors_disabled = parse.isFlagSet("ccd");
	if(console_colors_disabled) yafaray_setConsoleLogColorsEnabled(yi, YAFARAY_BOOL_FALSE);
	else yafaray_setConsoleLogColorsEnabled(yi, YAFARAY_BOOL_TRUE);

	const std::string verb_level = parse.getOptionString("vl");
	if(verb_level.empty()) yafaray_setConsoleVerbosityLevel(yi, YAFARAY_LOG_LEVEL_INFO);
	else yafaray_setConsoleVerbosityLevel(yi, yafaray_logLevelFromString(verb_level.c_str()));

	const std::string log_verb_level = parse.getOptionString("lvl");
	if(log_verb_level.empty()) yafaray_setConsoleVerbosityLevel(yi, YAFARAY_LOG_LEVEL_VERBOSE);
	else yafaray_setLogVerbosityLevel(yi, yafaray_logLevelFromString(verb_level.c_str()));

	std::string input_color_space_string = parse.getOptionString("ics");
	if(input_color_space_string.empty()) input_color_space_string = "LinearRGB";
	float input_gamma = static_cast<float>(parse.getOptionFloat("ig"));
	if(isNan_global(input_gamma)) input_gamma = 1.f;
	yafaray_setInputColorSpace(yi, input_color_space_string.c_str(), input_gamma);

	const std::vector<std::string> files = parse.getCleanArgs();
	if(files.empty()) return 0;
	const std::string xml_file_path = files.at(0);

	const std::string log_file_types = parse.getOptionString("l");
	bool save_txt = false;
	bool save_html = false;
	if(log_file_types == "txt") save_txt = true;
	else if(log_file_types == "html") save_html = true;
	else if(log_file_types == "txt+html") { save_txt = true; save_html = true; }
	yafaray_paramsSetBool(yi, "logging_save_txt", static_cast<yafaray_bool_t>(save_txt));
	yafaray_paramsSetBool(yi, "logging_save_html", static_cast<yafaray_bool_t>(save_html));

	yafaray_setInteractive(yi, YAFARAY_BOOL_FALSE);
	yafaray_xml_Parse(yi, xml_file_path.c_str());
	yafaray_render(yi, nullptr, nullptr, YAFARAY_DISPLAY_CONSOLE_NORMAL);
	yafaray_destroyInterface(yi);
	return 0;
}
