/*
MicroBuild
Copyright (C) 2016 TwinDrills

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Core/Platform/Path.h"

#include "FreeImage.h"

#include <memory>
#include <string>
#include <vector>

namespace MicroBuild {

// Represents a bitmap image and provides functionality to 
// load/save and modify said bitmap.
class Image
{
public:
	typedef std::shared_ptr<Image> Ptr;

private:
	FIBITMAP* m_bitmap;
	std::vector<Ptr> m_subImages;

protected:

public:	
	Image(FIBITMAP* Bitmap);
	Image();
	~Image();

	// Loads the image from the given url. 
	// Returns true on success.
	bool Load(const Platform::Path& Url);

	// Save the image to the given url, format is based
	// on the file extension.
	// Returns true on success.
	bool Save(const Platform::Path& Url);

	// Adds a sub image, only valid for animated images
	// or icon sets.
	void AddSubImage(Ptr SubImage);

	// Creates a copy of the image scaled to the given dimensions.
	Ptr Scale(int Width, int Height);

	// Gets width of image.
	int GetWidth();

	// Gets height of image.
	int GetHeight();

	// Converts a set of image files into a single icon file.
	static Image::Ptr CreateIcon(const std::vector<Platform::Path>& icons);

};

}; // namespace MicroBuild
