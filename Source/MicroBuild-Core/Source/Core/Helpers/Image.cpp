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

#include "PCH.h"
#include "Core/Helpers/Image.h"

#include <algorithm>
#include <cstdlib>

namespace MicroBuild {

Image::Image()
	: m_bitmap(nullptr)
{
}

Image::Image(FIBITMAP* Bitmap)
	: m_bitmap(Bitmap)
{
}

Image::~Image()
{
	if (m_bitmap)
	{
		FreeImage_Unload(m_bitmap);
	}
}

bool Image::Load(const Platform::Path& Url)
{
    FREE_IMAGE_FORMAT format = FIF_UNKNOWN;

	format = FreeImage_GetFileType(Url.ToString().c_str(), 0);
    if (format == FIF_UNKNOWN)
	{
        format = FreeImage_GetFIFFromFilename(Url.ToString().c_str());
	    if (format == FIF_UNKNOWN)
		{
		    return false;
		}
	}
    
	m_bitmap = FreeImage_Load(format, Url.ToString().c_str());
	if (!m_bitmap)
	{
		return false;
	}

	return true;
}

bool Image::Save(const Platform::Path& Url)
{	
	FREE_IMAGE_FORMAT format = FIF_UNKNOWN;

    format = FreeImage_GetFIFFromFilename(Url.ToString().c_str());
	if (format == FIF_UNKNOWN)
	{
		return false;
	}

	if (m_subImages.size() > 0)
	{
		FIMULTIBITMAP* bitmap = FreeImage_OpenMultiBitmap(format, Url.ToString().c_str(), true, false);
		if (!bitmap)
		{
			return false;
		}

		for (auto& subImage : m_subImages)
		{
			FreeImage_AppendPage(bitmap, subImage->m_bitmap);
		}

		if (m_bitmap)
		{
			FreeImage_AppendPage(bitmap, m_bitmap);
		}
	
		BOOL bResult = FreeImage_CloseMultiBitmap(bitmap);

		if (!bResult)
		{
			return false;
		}
	}
	else if (m_bitmap)
	{
		BOOL bResult = FreeImage_Save(format, m_bitmap, Url.ToString().c_str());
		if (!bResult)
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

void Image::AddSubImage(Ptr SubImage)
{
	m_subImages.push_back(SubImage);
}

Image::Ptr Image::Scale(int Width, int Height)
{
	FIBITMAP* newImage = FreeImage_Rescale(m_bitmap, Width, Height, FILTER_BICUBIC);
	if (!newImage)
	{
		return nullptr;
	}

	return std::make_shared<Image>(newImage);
}

int Image::GetWidth()
{
	if (!m_bitmap)
	{
		assert(false);
		return 0;
	}
	return FreeImage_GetWidth(m_bitmap);
}

int Image::GetHeight()
{
	if (!m_bitmap)
	{
		assert(false);
		return 0;
	}
	return FreeImage_GetHeight(m_bitmap);
}

Image::Ptr Image::CreateIcon(const std::vector<Platform::Path>& icons)
{
	std::vector<Image::Ptr> iconPool;
	for (auto& path : icons)
	{
		Image::Ptr subImage = std::make_shared<Image>();
		if (!subImage->Load(path))
		{
			return nullptr;
		}
		iconPool.push_back(subImage);
	}
	
	Image::Ptr iconImage = std::make_shared<Image>();
	for (int size = 16; size <= 256; size *= 2)
	{
		Image::Ptr subIcon = nullptr;
		
		// Try and find closest sized image from pool.
		for (auto& icon : iconPool)
		{
			if (subIcon == nullptr || abs(icon->GetWidth() - size) < abs(subIcon->GetWidth() - size))
			{
				subIcon = icon;
			}
		}
		
		if (!subIcon)
		{
			return nullptr;
		}

		// If it's not exactly the size we want, resize it.
		if (subIcon->GetWidth() != size || subIcon->GetHeight() != size)
		{
			subIcon = subIcon->Scale(size, size);
		}

		iconImage->AddSubImage(subIcon);
	}

	return iconImage;
}

}; // namespace MicroBuild
