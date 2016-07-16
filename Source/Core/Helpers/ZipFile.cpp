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

#include "PCH.h"
#include "Core/Helpers/Strings.h"
#include "Core/Helpers/ZipFile.h"

#include <algorithm>

namespace MicroBuild {

ZipFile::ZipFile()
{
}

ZipFile::~ZipFile()
{
}

bool ZipFile::AddDirectory(const Platform::Path& source, const Platform::Path& destination)
{
	std::vector<std::string> files = source.GetFiles();
	for (auto file : files)
	{
		Platform::Path src = source.AppendFragment(file, true);
		Platform::Path dest = destination.AppendFragment(file, true);

		if (!AddFile(src, dest))
		{
			return false;
		}
	}

	std::vector<std::string> dirs = source.GetDirectories();
	for (auto dir : dirs)
	{
		Platform::Path src = source.AppendFragment(dir, true);
		Platform::Path dest = destination.AppendFragment(dir, true);

		if (!AddDirectory(src, dest))
		{
			return false;
		}
	}

	return true;
}

bool ZipFile::AddFile(const Platform::Path& source, const Platform::Path& destination)
{
	BinaryStream sourceStream;
	if (!sourceStream.Open(source, false))
	{
		Log(LogSeverity::Info, "Failed to open: %s\n", source.ToString().c_str());
		return false;
	}

	sourceStream.Seek(0);
	uint32_t crc32 = sourceStream.Crc32();
	uint32_t fileSize = sourceStream.Length();
	std::string name = destination.ToString();

	uint32_t blockOffset = m_stream.Offset();;

	m_stream.Write<uint32_t>(0x04034b50);						// local file header signature
	m_stream.Write<uint16_t>(10);								// version needed to extract
	m_stream.Write<uint16_t>(0);								// general purpose bit flag 
	m_stream.Write<uint16_t>(0);								// compression method      
	m_stream.Write<uint16_t>(0);								// last mod file time      
	m_stream.Write<uint16_t>(0);								// last mod file date      
	m_stream.Write<uint32_t>(crc32);							// crc - 32               
	m_stream.Write<uint32_t>(fileSize);							// compressed size        
	m_stream.Write<uint32_t>(fileSize);							// uncompressed size        
	m_stream.Write<uint16_t>(name.size());						// file name length       
	m_stream.Write<uint16_t>(0);								// extra field length     
	m_stream.WriteBuffer(name.data(), name.size());				// file name(variable size)	
																// extra field(variable size)
	sourceStream.Seek(0);
	sourceStream.CopyTo(m_stream);								// file data

	// Store block.
	ZipFileBlock block;
	block.offset = blockOffset;
	block.crc32 = crc32;
	block.fileSize = fileSize;
	block.name = name;
	block.source = source;
	block.destination = destination;
	m_blocks.push_back(block);

	return true;
}

bool ZipFile::Open(const Platform::Path& path)
{
	m_blocks.clear();
	return m_stream.Open(path, true);
}

void ZipFile::Close()
{
	// Write out header information.
	uint32_t centralDirectoryOffset = m_stream.Offset();

	for (auto block : m_blocks)
	{
		m_stream.Write<uint32_t>(0x02014b50);						// local file header signature
		m_stream.Write<uint16_t>(10);								// version made by 
		m_stream.Write<uint16_t>(10);								// version needed to extract
		m_stream.Write<uint16_t>(0);								// general purpose bit flag 
		m_stream.Write<uint16_t>(0);								// compression method      
		m_stream.Write<uint16_t>(0);								// last mod file time      
		m_stream.Write<uint16_t>(0);								// last mod file date      
		m_stream.Write<uint32_t>(block.crc32);						// crc - 32                  
		m_stream.Write<uint32_t>(block.fileSize);					// compressed size        
		m_stream.Write<uint32_t>(block.fileSize);					// uncompressed size    
		m_stream.Write<uint16_t>(block.name.size());				// file name length      
		m_stream.Write<uint16_t>(0);								// extra field length             
		m_stream.Write<uint16_t>(0);								// file comment length        
		m_stream.Write<uint16_t>(0);								// disk number start              
		m_stream.Write<uint16_t>(0);								// internal file attributes       
		m_stream.Write<uint32_t>(0);								// external file attributes       
		m_stream.Write<uint32_t>(block.offset);						// relative offset of local header 4 bytes
		m_stream.WriteBuffer(block.name.data(), block.name.size());	// file name(variable size)	
																	// extra field(variable size)
																	// file comment(variable size)
	}

	uint32_t centralDirectorySize = m_stream.Offset() - centralDirectoryOffset;

	// Write out end of central directory.	
	m_stream.Write<uint32_t>(0x06054b50);				// End of central directory signature
	m_stream.Write<uint16_t>(0);						// Number of this disk
	m_stream.Write<uint16_t>(0);						// Disk where central directory starts
	m_stream.Write<uint16_t>(m_blocks.size());			// Number of central directory records on this disk
	m_stream.Write<uint16_t>(m_blocks.size());			// Total number of central directory records
	m_stream.Write<uint32_t>(centralDirectorySize);		// Size of central directory(bytes)
	m_stream.Write<uint32_t>(centralDirectoryOffset);	// Offset of start of central directory, relative to start of archive
	m_stream.Write<uint16_t>(0);						// Comment length(n)
														// Comment

	m_stream.Close();
}

}; // namespace MicroBuild