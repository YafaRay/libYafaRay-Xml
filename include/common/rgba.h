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

#ifndef LIBYAFARAY_XML_RGBA_H
#define LIBYAFARAY_XML_RGBA_H

namespace yafaray_xml
{

struct Rgba
{
	explicit Rgba(float v) : Rgba(v, 1.f) { }
	explicit Rgba(float v, float a) : Rgba(v, v, v, a) { }
	explicit Rgba(float r, float g, float b, float a) : r_(r), g_(g), b_(b), a_(a) { }
	float r_, g_, b_, a_;
};

} //namespace yafaray_xml

#endif //LIBYAFARAY_XML_RGBA_H
