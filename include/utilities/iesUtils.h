/****************************************************************************
 * 		iesUtils.h: utilities for parsing IES data files
 *      This is part of the yafaray package
 *      Copyright (C) 2009  Bert Buchholz and Rodrigo Placencia
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

#ifndef IESUTILS_H
#define IESUTILS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <utilities/curveUtils.h>

__BEGIN_YAFRAY
//!TODO: Preblur data

#define TYPE_C 1
#define TYPE_B 2
#define TYPE_A 3

class IESData_t
{
public:

	IESData_t(float blur, int reso);
	~IESData_t();
	bool parseIESFile(const std::string file);
	float getRadianceBlurred(float u, float v) const;
	float getMaxVAngle() const { return maxVAngle; }
	
private:

	bool getRadiance(float hAng, float vAng, float& rad) const;
	float blurRadiance(float hAng, float vAng) const;
	
	float* vertAngleMap; //<! vertical spherical angles
	float* horAngleMap; //<! horizontal sperical angles
	float** radMap; //<! spherical radiance map corresponding with entries to the angle maps
	int horAngles; //<! number of angles in the 2 directions
	int vertAngles;
	
	float maxRad;
	float resStep;
	float resBound;
	
	float maxVAngle;
	
	int type;
	
	bool blurred;
};

IESData_t::IESData_t(float blur, int reso)
{
	blurred = !(blur < 0.5f || reso < 2);
	
	if(blurred)
	{
		resStep = blur/(float)reso;
		resBound = (float)reso * 0.5f;
	}
}

IESData_t::~IESData_t()
{
	if(vertAngleMap) delete [] vertAngleMap;
	if(horAngleMap) delete [] horAngleMap;
	if(radMap)
	{
		for(int i = 0; i < horAngles; i++) delete [] radMap[i];
		delete [] radMap;
	}
}

float IESData_t::getRadianceBlurred(float hAng, float vAng) const
{

	float ret = 0.f;

	if (!blurred)
	{
		getRadiance(hAng, vAng, ret);
	}
	else
	{
		ret = blurRadiance(hAng, vAng);
	}

	return (ret * maxRad);
}

//! hAng and vAng in degrees, rad is the buffer for the radiance at that angle
bool IESData_t::getRadiance(float h, float v, float& rad) const {
	
	int x = 0, y = 0;
	rad = 0.f;
	float hAng = 0.f, vAng = 0.f;
	
	if(type == TYPE_C)
	{
		hAng = h;
		vAng = v;
	}
	else
	{
		hAng = v;
		vAng = h;
		if(type == TYPE_B)
		{
			hAng += 90;
			if(hAng > 360.f) hAng -= 360.f;
		}
	}

	if(hAng > 180.f && horAngleMap[horAngles-1] <= 180.f) hAng = 360.f - hAng;
	if(hAng > 90.f && horAngleMap[horAngles-1] <= 90.f) hAng -= 90.f;
	
	if(vAng > 90.f && vertAngleMap[vertAngles-1] <= 90.f) vAng -= 90.f;
	
	for(int i = 0;i < horAngles; i++)
	{
		if(horAngleMap[i] <= hAng && horAngleMap[i+1] > hAng)
		{
			x = i;
		}
	}

	for(int i = 0;i < vertAngles; i++)
	{
		if(vertAngleMap[i] <= vAng && vertAngleMap[i+1] > vAng)
		{
			y = i;
			break;
		}
	}
	
	
	if(hAng == horAngleMap[x] && vAng == vertAngleMap[y])
	{
		rad = radMap[x][y];
	}
	else
	{
		int x1 = x, x2 = x+1;
		int y1 = y, y2 = y+1;
		
		float dX = (hAng - horAngleMap[x1]) / (horAngleMap[x2] - horAngleMap[x1]);
		float dY = (vAng - vertAngleMap[y1]) / (vertAngleMap[y2] - vertAngleMap[y1]);
		
		float rx1 = ((1.f - dX) * radMap[x1][y1]) + (dX * radMap[x2][y1]);
		float rx2 = ((1.f - dX) * radMap[x1][y2]) + (dX * radMap[x2][y2]);
		
		rad = ((1.f - dY) * rx1) + (dY * rx2);
		if(false)
		{
			Y_INFO << "rad:" << rad;
			std::cout << " | dX:" << dX << " dY: " << dY;
			std::cout << " | Angles (h,x1,x2) | (v,y1,y2): (" << hAng << ", " << horAngleMap[x1] << ", " << horAngleMap[x2] << ") | (" << vAng << ", " << vertAngleMap[y1] << ", " << vertAngleMap[y2] << ")";
			std::cout << " rx1:" << rx1 << " rx2: " << rx2 << "\n";
		}
	}

	
	return true;
}

float IESData_t::blurRadiance(float hAng, float vAng) const
{
	float ret = 0.f;
	int hits = 0;
	for (float i = -resBound; i < resBound; i += resStep)
	{
		float tmp;
		
		if (getRadiance(hAng + resStep , vAng + resStep, tmp))
		{
			ret += tmp;
			hits++;
		}
	}

	return (hits > 0) ? ret / (float)hits : 0.f;
}

// IES description: http://lumen.iee.put.poznan.pl/kw/iesna.txt

bool IESData_t::parseIESFile(const std::string iesFile)
{
	using namespace std;
	
	Y_INFO << "IES Parser: Parsing IES file " << iesFile << std::endl;
	
	ifstream fin(iesFile.c_str(), std::ios::in);
	
	if (!fin)
	{
		Y_ERROR << "IES Parser: Could not open IES file: " << iesFile << std::endl;
		return false;
	}

	string line;
	string dummy;
	
	fin >> line;
	
	while (line.find("TILT=") == string::npos)
	{
		fin >> line;
	}
	
	if(line.find("TILT=") != string::npos)
	{
		if(line == "TILT=INCLUDE")
		{
			Y_INFO << "IES Parser: Tilt data included in IES file.\nSkiping...\n";
			
			int pairs = 0;
			
			fin >> line;
			fin >> pairs;
			
			for(int i = 0; i < (pairs * 2); i++) fin >> line;
			
			Y_INFO << "IES Parser: Tilt data skipped.\n";
		}
		else if(line == "TILT=NONE")
		{
			Y_INFO << "IES Parser: No tilt data.\n";
		}
		else if(line == "TILT=NONE")
		{
			Y_INFO << "IES Parser: Tilt data in another file.\n";
		}
	}
	else
	{
		fin.close();
	
		Y_INFO << "IES Parser: Tilt not found IES invalid!\n";
		
		return false;
	}
	
	float candelaMult = 0.f;
	
	fin >> line;
	Y_INFO << "IES Parser: Number of lamps: " << line << "\n";
	fin >> line;
	Y_INFO << "IES Parser: lumens per lamp: " << line << "\n";
	fin >> candelaMult;
	candelaMult *= 0.001;
	Y_INFO << "IES Parser: Candela multiplier (kcd): " << candelaMult << "\n";
	fin >> vertAngles;
	Y_INFO << "IES Parser: Vertical Angles: " << vertAngles << "\n";
	fin >> horAngles;
	Y_INFO << "IES Parser: Horizontal Angles: " << horAngles << "\n";
	type = 0;
	fin >> type;
	Y_INFO << "IES Parser: Photometric Type: " << type << "\n";
	fin >> line;
	Y_INFO << "IES Parser: Units Type: " << line << "\n";
	
	float w = 0.f, l = 0.f, h = 0.f;
	
	fin >> w;
	fin >> l;
	fin >> h;

	Y_INFO << "IES Parser: Luminous opening dimensions:\n";
	Y_INFO << "IES Parser: (Width, Length, Height) = (" << w << ", " << l << ", " << h << ")\n";
	Y_INFO << "IES Parser: Lamp Geometry: ";
	
	//Check geometry type
	if(w == 0.f && l == 0.f && h == 0.f)
	{
		Y_INFO << "Point Light\n";
	}
	else if(w >= 0.f && l >= 0.f && h >= 0.f)
	{
		Y_INFO << "Rectangular Light\n";
	}
	else if(w < 0.f && l == 0.f && h == 0.f)
	{
		Y_INFO << "Circular Light\n";
	}
	else if(w < 0.f && l == 0.f && h < 0.f)
	{
		Y_INFO << "Shpere Light\n";
	}
	else if(w < 0.f && l == 0.f && h >= 0.f)
	{
		Y_INFO << "Vertical Cylindric Light\n";
	}
	else if(w == 0.f && l >= 0.f && h < 0.f)
	{
		Y_INFO << "Horizontal Cylindric Light (Along width)\n";
	}
	else if(w >= 0.f && l == 0.f && h < 0.f)
	{
		Y_INFO << "Horizontal Cylindric Light (Along length)\n";
	}
	else if(w < 0.f && l >= 0.f && h >= 0.f)
	{
		Y_INFO << "Elipse Light (Along width)\n";
	}
	else if(w >= 0.f && l < 0.f && h >= 0.f)
	{
		Y_INFO << "Elipse Light (Along length)\n";
	}
	else if(w < 0.f && l >= 0.f && h < 0.f)
	{
		Y_INFO << "Elipsoid Light (Along width)\n";
	}
	else if(w >= 0.f && l < 0.f && h < 0.f)
	{
		Y_INFO << "Elipsoid Light (Along length)\n";
	}
	
	fin >> line;
	Y_INFO << "IES Parser: Ballast Factor: " << line << "\n";
	fin >> line;
	Y_INFO << "IES Parser: Ballast-Lamp Photometric Factor: " << line << "\n";
	fin >> line;
	Y_INFO << "IES Parser: Input Watts: " << line << "\n";

	vertAngleMap = new float[vertAngles];
	
	maxVAngle = 0.f;

	Y_INFO << "IES Parser: Vertical Angle Map:\n";

	for (int i = 0; i < vertAngles; ++i)
	{
		fin >> vertAngleMap[i];
		if(maxVAngle < vertAngleMap[i]) maxVAngle = vertAngleMap[i];
		std::cout << vertAngleMap[i] << ", ";
	}
	
	std::cout << std::endl;

	if(vertAngleMap[0] > 0.f)
	{
		Y_INFO << "IES Parser: Vertical Angle Map (transformed):\n";
		float minus = vertAngleMap[0];
		maxVAngle -= minus;
		for (int i = 0; i < vertAngles; ++i)
		{
			vertAngleMap[i] -= minus;
			std::cout << vertAngleMap[i] << ", ";
		}
		std::cout << std::endl;
	}
	
	Y_INFO << "IES Parser: Max vertical angle (degrees): " << maxVAngle << "\n";
	
	maxVAngle = degToRad(maxVAngle);

	Y_INFO << "IES Parser: Max vertical angle (radians): " << maxVAngle << "\n";
	
	bool hAdjust = false;
	
	if(type == TYPE_C && horAngles == 1)
	{
		horAngles++;
		hAdjust = true;
	}
	
	horAngleMap = new float[horAngles];
	
	Y_INFO << "IES Parser: Horizontal Angle Map:\n";
	
	for (int i = 0; i < horAngles; ++i)
	{
		if(i == horAngles - 1 && hAdjust) horAngleMap[i] = 180.f;
		else fin >> horAngleMap[i];
		std::cout << horAngleMap[i] << ", ";
	}
	std::cout << std::endl;

	maxRad = 0.f;
	
	radMap = new float*[horAngles];
	for (int i = 0; i < horAngles; ++i)
	{
		radMap[i] = new float[vertAngles];
		for (int j = 0; j < vertAngles; ++j)
		{
			if(i == horAngles - 1 && hAdjust) radMap[i][j] = radMap[i-1][j];
			else  fin >> radMap[i][j];
			if(maxRad < radMap[i][j]) maxRad = radMap[i][j];
		}
	}
	
	Y_INFO << "IES Parser: maxRad = " << maxRad << "\n";
	maxRad = 1.f / maxRad;
	
	fin.close();
	
	Y_INFO << "IES Parser: IES File parsed successfully\n";
	
	return true;
}

__END_YAFRAY

#endif //IESUTILS_H
