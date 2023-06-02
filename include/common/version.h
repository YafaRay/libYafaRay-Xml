#pragma once
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

#ifndef LIBYAFARAY_XML_VERSION_H
#define LIBYAFARAY_XML_VERSION_H

#include <string>

namespace yafaray_xml
{

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

} //namespace yafaray_xml

#endif //LIBYAFARAY_XML_VERSION_H
