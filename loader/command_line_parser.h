#pragma once
/****************************************************************************
 *      console_utils.h: General command line parsing and other utilities (soon)
 *      This is part of the libYafaRay-Xml package
 *      Copyright (C) 2010  Rodrigo Placencia
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

#ifndef YAFARAY_CONSOLE_H
#define YAFARAY_CONSOLE_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <limits>

//! Struct that holds the state of the registered options and the values parsed from command line args
class CliParserOption final
{
	public:
		CliParserOption(const std::string &s_opt, const std::string &l_opt, bool is_flag, const std::string &desc);
		std::string getShortOpt() const { return short_opt_; }
		std::string getLongOpt() const { return long_opt_; }
		bool isFlag() const { return is_flag_; }
		bool isSet() const { return is_set_; }
		std::string getValue() const { return value_; }
		std::string getDescription() const { return desc_; }
		void setValue(const std::string &value) { value_ = value; }
		void markAsSet(bool value) { is_set_ = value; }

	private:
		std::string short_opt_;
		std::string long_opt_;
		bool is_flag_ = false;
		std::string desc_;
		std::string value_;
		bool is_set_ = false;
};

inline CliParserOption::CliParserOption(const std::string &s_opt, const std::string &l_opt, bool is_flag, const std::string &desc) : short_opt_(s_opt), long_opt_(l_opt), is_flag_(is_flag), desc_(desc)
{
	if(!short_opt_.empty()) short_opt_ = "-" + s_opt;
	if(!long_opt_.empty()) long_opt_ = "--" + l_opt;
}

//! The command line option parsing and handling class
/*!parses GNU style command line arguments pairs and flags with space (' ') as pair separator*/

class CliParser final
{
	public:
		CliParser() = default; //! Default constructor for 2 step init
		CliParser(int argc, char **argv, int clean_args_num, int clean_opt_args_num, const std::string &clean_arg_error);  //! One step init constructor
		void setCommandLineArgs(int argc, char **argv);  //! Initialization method for 2 step init
		void setCleanArgsNumber(int arg_num, int opt_arg, const std::string &clean_arg_error); //! Configures the parser to get arguments non-paired at the end of the command string with optional arg number
		void setOption(const std::string &s_opt, const std::string &l_opt, bool is_flag, const std::string &desc); //! Option registrar method, it adds a valid parsing option to the list
		const CliParserOption *findOption(const std::string &s_opt, const std::string &l_opt = "") const;
		std::string getOptionString(const std::string &s_opt, const std::string &l_opt = "") const; //! Retrieves the string value associated with the option if any, if no option returns an empty string
		int getOptionInteger(const std::string &s_opt, const std::string &l_opt = "") const; //! Retrieves the integer value associated with the option if any, if no option returns std::numeric_limits<int>::min()
		double getOptionFloat(const std::string &s_opt, const std::string &l_opt = "") const; //! Retrieves the floating point value associated with the option if any, if no option returns std::numeric_limits<double>::quiet_NaN();
		bool isFlagSet(const std::string &s_opt, const std::string &l_opt = "") const; //! Returns true is the flag was set in command line, false else
		std::vector<std::string> getCleanArgs() const;
		void setAppName(const std::string &name, const std::string &b_usage);
		void printUsage() const; //! Prints usage instructions with the registrered options
		void printError() const; //! Prints error found during parsing (if any)
		bool parseCommandLine(); //! Parses the input values from command line and fills the values on the right options if they exist on the args

	private:
		size_t arg_count_ = 0; //! Input arguments count
		std::string app_name_; //! Holds the app name used in the usage construction, defaults to argv[0]
		std::string bin_name_; //! Holds the name of the executabl binary (argv[0])
		std::string basic_usage_; //! Holds the basic usage instructions of the command
		std::vector<std::string> arg_values_; //! Holds argv values
		std::vector<std::string> clean_values_; //! Holds clean (non-paired options) values
		std::vector<std::unique_ptr<CliParserOption>> reg_options_; //! Holds registrered options
		size_t clean_args_ = 0;
		size_t clean_args_optional_ = 0;
		std::string clean_args_error_;
		std::string parse_error_;
};

inline CliParser::CliParser(int argc, char **argv, int clean_args_num, int clean_opt_args_num, const std::string &clean_arg_error)
{
	setCommandLineArgs(argc, argv);
	setCleanArgsNumber(clean_args_num, clean_opt_args_num, clean_arg_error);
}

inline void CliParser::setCommandLineArgs(int argc, char **argv)
{
	arg_count_ = static_cast<size_t>(argc);
	arg_values_.clear();
	app_name_ = std::string(argv[0]);
	bin_name_ = std::string(argv[0]);
	for(size_t i = 1; i < arg_count_; i++)
	{
		arg_values_.emplace_back(std::string(argv[i]));
	}
}

inline void CliParser::setCleanArgsNumber(int arg_num, int opt_arg, const std::string &clean_arg_error)
{
	clean_args_ = arg_num;
	clean_args_optional_ = opt_arg;
	clean_args_error_ = clean_arg_error;
}

inline void CliParser::setOption(const std::string &s_opt, const std::string &l_opt, bool is_flag, const std::string &desc)
{
	if(!s_opt.empty() || !l_opt.empty()) reg_options_.emplace_back(new CliParserOption(s_opt, l_opt, is_flag, desc));
}

inline const CliParserOption *CliParser::findOption(const std::string &s_opt, const std::string &l_opt) const
{
	const std::string cmp_s_opt = "-" + s_opt;
	const std::string cmp_l_opt = "--" + l_opt;

	for(const auto &reg_option : reg_options_)
	{
		if(reg_option->getShortOpt() == cmp_s_opt || reg_option->getLongOpt() == cmp_l_opt)
		{
			return reg_option.get();
		}
	}
	return nullptr;
}

inline std::string CliParser::getOptionString(const std::string &s_opt, const std::string &l_opt) const
{
	const CliParserOption *reg_option = findOption(s_opt, l_opt);
	if(!reg_option || reg_option->isFlag()) return "";
	else return reg_option->getValue();
}

inline int CliParser::getOptionInteger(const std::string &s_opt, const std::string &l_opt) const
{
	constexpr int default_value = std::numeric_limits<int>::min();
	const std::string option_value_string = getOptionString(s_opt, l_opt);
	if(option_value_string.empty()) return default_value;
	std::istringstream string_to_value_ss(option_value_string);
	int result;
	if(string_to_value_ss >> result) return result;
	else return default_value;
}

inline double CliParser::getOptionFloat(const std::string &s_opt, const std::string &l_opt) const
{
	constexpr double default_value = std::numeric_limits<double>::quiet_NaN();
	const std::string option_value_string = getOptionString(s_opt, l_opt);
	if(option_value_string.empty()) return default_value;
	std::istringstream string_to_value_ss(option_value_string);
	double result;
	if(string_to_value_ss >> result) return result;
	else return default_value;
}

inline bool CliParser::isFlagSet(const std::string &s_opt, const std::string &l_opt) const
{
	const CliParserOption *reg_option = findOption(s_opt, l_opt);
	if(!reg_option || !reg_option->isFlag()) return "";
	else return reg_option->isSet();
}

inline std::vector<std::string> CliParser::getCleanArgs() const
{
	return clean_values_;
}

inline void CliParser::setAppName(const std::string &name, const std::string &b_usage)
{
	app_name_.clear();
	app_name_ = name;
	basic_usage_.clear();
	basic_usage_ = b_usage;
}

inline void CliParser::printUsage() const
{
	std::cout << app_name_ << std::endl
		   << "Usage: " << bin_name_ << " " << basic_usage_ << std::endl
		   << "OPTIONS:" << std::endl;
	for(const auto &reg_option : reg_options_)
	{
		std::stringstream name;
		name << reg_option->getShortOpt() << ", " << reg_option->getLongOpt() << (reg_option->isFlag() ? "" : " <value>");
		std::cout << "    "
				  << std::setiosflags(std::ios::left) << std::setw(35)
				  << name.str()
				  << reg_option->getDescription() << std::endl;
	}
	std::cout << "Usage instructions end." << std::endl;
}

inline void CliParser::printError() const
{
	std::cout << parse_error_ << std::endl;
}

inline bool CliParser::parseCommandLine()
{
	std::stringstream error;
	clean_values_.clear();
	for(size_t i = 0; i < arg_values_.size(); i++)
	{
		if((i >= arg_values_.size() - (clean_args_ - clean_args_optional_)) || (i >= arg_values_.size() - clean_args_))
		{
			if(arg_values_[i].compare(0, 1, "-") != 0)
			{
				clean_values_.push_back(arg_values_[i]);
				continue;
			}
		}

		for(const auto &reg_option : reg_options_)
		{
			if(reg_option->getShortOpt() == arg_values_[i] || reg_option->getLongOpt() == arg_values_[i])
			{
				if(!reg_option->isFlag())
				{
					if(i < arg_values_.size())
					{
						i++;
						if(arg_values_[i].compare(0, 1, "-") != 0)
						{
							reg_option->setValue(arg_values_[i]);
							reg_option->markAsSet(true);
						}
						else
						{
							error << "Option " << reg_option->getLongOpt() << " has no value";
							parse_error_ = error.str();
							return false;
						}
					}
					else
					{
						error << "Option " << reg_option->getLongOpt() << " has no value";
						parse_error_ = error.str();
						return false;
					}
				}
				else
				{
					reg_option->markAsSet(true);
				}
			}
		}
	}

	if(clean_values_.size() < clean_args_ && clean_values_.size() < clean_args_ - clean_args_optional_)
	{
		error << clean_args_error_;
		parse_error_ = error.str();
		return false;
	}
	return true;
}

#endif //YAFARAY_CONSOLE_H
