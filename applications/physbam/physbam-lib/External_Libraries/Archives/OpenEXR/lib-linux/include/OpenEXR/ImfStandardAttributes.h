///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2003, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////


#ifndef INCLUDED_IMF_STANDARD_ATTRIBUTES_H
#define INCLUDED_IMF_STANDARD_ATTRIBUTES_H

//-----------------------------------------------------------------------------
//
//	Optional Standard Attributes -- these attributes are "optional"
//	because not every image file header has them, but they define a
//	"standard" way to represent commonly used data in the file header.
//
//	For each attribute, with name "foo", and type "T", the following
//	functions are automatically generated via macros:
//
//	void			   addFoo (Header &header, const T &value);
//	bool			   hasFoo (const Header &header);
//	const TypedAttribute<T> &  fooAttribute (const Header &header);
//	TypedAttribute<T> &	   fooAttribute (Header &header);
//	const T &		   foo (const Header &Header);
//	T &			   foo (Header &Header);
//
//-----------------------------------------------------------------------------

#include <ImfHeader.h>
#include <ImfFloatAttribute.h>
#include <ImfStringAttribute.h>
#include <ImfChromaticitiesAttribute.h>


#define IMF_STD_ATTRIBUTE_DEF(name,suffix,type)				      \
									      \
    void			 add##suffix (Header &header, const type &v); \
    bool			 has##suffix (const Header &header);	      \
    const TypedAttribute<type> & name##Attribute (const Header &header);      \
    TypedAttribute<type> &	 name##Attribute (Header &header);	      \
    const type &		 name (const Header &header);		      \
    type &			 name (Header &header);


namespace Imf {

//
// chromaticities -- for RGB images, specifies the CIE (x,y)
// chromaticities of the primaries and the white point
//

IMF_STD_ATTRIBUTE_DEF (chromaticities, Chromaticities, Chromaticities)


//
// whiteLuminance -- for RGB images, defines the luminance, in Nits
// (candelas per square meter) of the RGB value (1.0, 1.0, 1.0).
//
// If the chromaticities and the whiteLuminance of an RGB image are
// known, then it is possible to convert the image's pixels from RGB
// to CIE XYZ tristimulus values (see function RGBtoXYZ() in header
// file ImfChromaticities.h).
// 
//

IMF_STD_ATTRIBUTE_DEF (whiteLuminance, WhiteLuminance, float)


//
// xDensity -- horizontal output density, in pixels per inch.
// The image's vertical output density is xDensity * pixelAspectRatio.
//

IMF_STD_ATTRIBUTE_DEF (xDensity, XDensity, float)


//
// owner -- name of the owner of the image
//

IMF_STD_ATTRIBUTE_DEF (owner, Owner, std::string)
   

//
// comments -- additional image information in human-readable
// form, for example a verbal description of the image
//

IMF_STD_ATTRIBUTE_DEF (comments, Comments, std::string)


//
// capDate -- the date when the image was created or captured,
// in local time, and formatted as
//
//    YYYY:MM:DD hh:mm:ss
//
// where YYYY is the year (4 digits, e.g. 2003), MM is the month
// (2 digits, 01, 02, ... 12), DD is the day of the month (2 digits,
// 01, 02, ... 31), hh is the hour (2 digits, 00, 01, ... 23), mm
// is the minute, and ss is the second (2 digits, 00, 01, ... 59).
//
//

IMF_STD_ATTRIBUTE_DEF (capDate, CapDate, std::string)


//
// utcOffset -- offset of local time at capDate from
// Universal Coordinated Time (UTC), in seconds:
//
//    UTC == local time + utcOffset
//

IMF_STD_ATTRIBUTE_DEF (utcOffset, utcOffset, float)


//
// longitude, latitude, altitude -- for images of real objects, the
// location where the image was recorded.  Longitude and latitude are
// in degrees east of Greenwich and north of the equator.  Altitude
// is in meters above sea level.  For example, Kathmandu, Nepal is
// at longitude 85.317, latitude 27.717, altitude 1305.
//

IMF_STD_ATTRIBUTE_DEF (longitude, Longitude, float)
IMF_STD_ATTRIBUTE_DEF (latitude, Latitude, float)
IMF_STD_ATTRIBUTE_DEF (altitude, Altitude, float)


//
// focus -- the camera's focus distance, in meters
//

IMF_STD_ATTRIBUTE_DEF (focus, Focus, float)


//
// exposure -- exposure time, in seconds
//

IMF_STD_ATTRIBUTE_DEF (expTime, ExpTime, float)


//
// aperture -- the camera's lens aperture, in f-stops (focal length
// of the lens divided by the diameter of the iris opening)
//

IMF_STD_ATTRIBUTE_DEF (aperture, Aperture, float)


//
// isoSpeed -- the ISO speed of the film or image sensor
// that was used to record the image
//

IMF_STD_ATTRIBUTE_DEF (isoSpeed, IsoSpeed, float)


} // namespace Imf

#endif
