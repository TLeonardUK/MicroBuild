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
#include "Core/Helpers/Strings.h"
#include "Core/Helpers/ZipFile.h"

#include "zlib.h"

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

bool ZipFile::CompressFile(
	const Platform::Path& input, 
	const Platform::Path& output,
	uint64_t& inputSize,
	uint64_t& outputSize,
	uint32_t& inputCrc32)
{
	BinaryStream inputStream;
	BinaryStream outputStream;

	if (!inputStream.Open(input, false))
	{
		Log(LogSeverity::Info, "Failed to open: %s\n", input.ToString().c_str());
		return false;
	}

	inputCrc32 = inputStream.Crc32();
	inputSize = inputStream.Length();

	inputStream.Seek(0);

	if (!outputStream.Open(output, true))
	{
		Log(LogSeverity::Info, "Failed to open: %s\n", output.ToString().c_str());
		return false;
	}

	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = 0;
	strm.avail_in = 0;

	deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
	
	char* bufferIn = new char[k_zlibCompressBuffer];
	char* bufferOut = new char[k_zlibCompressBuffer];
	outputSize = 0;

	uint64_t remaining = inputSize;
	while (remaining > 0)
	{
		uint64_t chunkSize = remaining;
		if (chunkSize > k_zlibCompressBuffer)
		{
			chunkSize = k_zlibCompressBuffer;
		}

		inputStream.ReadBuffer(bufferIn, chunkSize);

		strm.next_in = (Bytef*)bufferIn;
		strm.avail_in = (uInt)chunkSize;

		do
		{
			strm.next_out = (Bytef*)bufferOut;
			strm.avail_out = k_zlibCompressBuffer;

			deflate(&strm, Z_PARTIAL_FLUSH);

			int totalBytes = k_zlibCompressBuffer - strm.avail_out;
			outputStream.WriteBuffer(bufferOut, totalBytes);
		}
		while (strm.avail_in > 0);

		remaining -= chunkSize;
	}

	{
		strm.next_out = (Bytef*)bufferOut;
		strm.avail_out = k_zlibCompressBuffer;

		deflate(&strm, Z_FINISH);

		int totalBytes = k_zlibCompressBuffer - strm.avail_out;
		outputStream.WriteBuffer(bufferOut, totalBytes);
	}

	deflateEnd(&strm);

	delete bufferIn;
	delete bufferOut;

	// Check output file size. (gzip has not 64bit ones?)
	outputSize = outputStream.Length();
	outputStream.Close();

	return true;
}

bool ZipFile::AddFile(const Platform::Path& source, const Platform::Path& destination)
{
	uint32_t crc32 = 0;
	uint64_t fileSize = 0;
	uint64_t compressedFileSize = 0;
	std::string name = destination.ToString();

	Platform::Path tempFile = m_path.AppendFragment(".compress.tmp");
	if (!CompressFile(source, tempFile, fileSize, compressedFileSize, crc32))
	{
		return false;
	}


	uint64_t blockOffset = m_stream.Offset();;

	m_stream.Write<uint32_t>(0x04034b50);						// local file header signature
	m_stream.Write<uint16_t>(k_zip64Version);					// version needed to extract
	m_stream.Write<uint16_t>(0);								// general purpose bit flag 
	m_stream.Write<uint16_t>(k_compressionMethod);				// compression method      
	m_stream.Write<uint16_t>(0);								// last mod file time      
	m_stream.Write<uint16_t>(0);								// last mod file date      
	m_stream.Write<uint32_t>(crc32);							// crc - 32               
	m_stream.Write<uint32_t>(0xFFFFFFFF);						// compressed size        
	m_stream.Write<uint32_t>(0xFFFFFFFF);						// uncompressed size        
	m_stream.Write<uint16_t>(name.size());						// file name length       
	m_stream.Write<uint16_t>(k_zip64HeaderSize);				// extra field length     
	m_stream.WriteBuffer(name.data(), name.size());				// file name(variable size)	

																// extra field(variable size)
	m_stream.Write<uint16_t>(0x0001);							// Tag for this "extra" block type
	m_stream.Write<uint16_t>(k_zip64HeaderSize - 4);			// Size       
	m_stream.Write<uint64_t>(fileSize);							// Original Size       
	m_stream.Write<uint64_t>(compressedFileSize);				// Compressed Size      
	m_stream.Write<uint64_t>(blockOffset);						// Relative Header Offset
	m_stream.Write<uint32_t>(0);								// Disk Start Number     

																// extra field(variable size)

	BinaryStream sourceStream;
	if (!sourceStream.Open(tempFile, false))
	{
		Log(LogSeverity::Info, "Failed to open: %s\n", tempFile.ToString().c_str());
		return false;
	}

	sourceStream.Seek(0);
	sourceStream.CopyTo(m_stream);								// file data
	sourceStream.Close();

	// Remove temporary file.
	tempFile.Delete();

	// Store block.
	ZipFileBlock block;
	block.offset = blockOffset;
	block.crc32 = crc32;
	block.fileSize = fileSize;
	block.compressedFileSize = compressedFileSize;
	block.name = name;
	block.source = source;
	block.destination = destination;
	m_blocks.push_back(block);

	return true;
}

bool ZipFile::Open(const Platform::Path& path)
{
	m_path = path;
	m_blocks.clear();
	return m_stream.Open(path, true);
}

void ZipFile::Close()
{
	// Write out header information.
	uint64_t centralDirectoryOffset = m_stream.Offset();

	for (auto block : m_blocks)
	{
		m_stream.Write<uint32_t>(0x02014b50);						// local file header signature
		m_stream.Write<uint16_t>(k_zip64Version);					// version made by 
		m_stream.Write<uint16_t>(k_zip64Version);					// version needed to extract
		m_stream.Write<uint16_t>(0);								// general purpose bit flag 
		m_stream.Write<uint16_t>(k_compressionMethod);				// compression method      
		m_stream.Write<uint16_t>(0);								// last mod file time      
		m_stream.Write<uint16_t>(0);								// last mod file date      
		m_stream.Write<uint32_t>(block.crc32);						// crc - 32                  
		m_stream.Write<uint32_t>(0xFFFFFFFF);						// compressed size        
		m_stream.Write<uint32_t>(0xFFFFFFFF);						// uncompressed size    
		m_stream.Write<uint16_t>(block.name.size());				// file name length      
		m_stream.Write<uint16_t>(k_zip64HeaderSize);				// extra field length             
		m_stream.Write<uint16_t>(0);								// file comment length        
		m_stream.Write<uint16_t>(0xFFFF);							// disk number start              
		m_stream.Write<uint16_t>(0);								// internal file attributes       
		m_stream.Write<uint32_t>(0);								// external file attributes       
		m_stream.Write<uint32_t>(0xFFFFFFFF);						// relative offset of local header 4 bytes
		m_stream.WriteBuffer(block.name.data(), block.name.size());	// file name(variable size)	
		
																	// extra field(variable size)
		m_stream.Write<uint16_t>(0x0001);							// Tag for this "extra" block type
		m_stream.Write<uint16_t>(k_zip64HeaderSize - 4);			// Size       
		m_stream.Write<uint64_t>(block.fileSize);					// Original Size       
		m_stream.Write<uint64_t>(block.compressedFileSize);			// Compressed Size      
		m_stream.Write<uint64_t>(block.offset);						// Relative Header Offset
		m_stream.Write<uint32_t>(0);								// Disk Start Number     

																	// file comment(variable size)
	}

	uint64_t centralDirectorySize = m_stream.Offset() - centralDirectoryOffset;
	uint64_t endOfCentralDirectoryRecord = m_stream.Offset();

	// zip64 end of central directory record.
	m_stream.Write<uint32_t>(0x06064b50);							// signature 
	m_stream.Write<uint64_t>(k_zip64CentralRecordSize);				// size of zip64 end of central directory record               
	m_stream.Write<uint16_t>(k_zip64Version);						// version made by         
	m_stream.Write<uint16_t>(k_zip64Version);						// version needed to extract      
	m_stream.Write<uint32_t>(0);									// number of this disk             
	m_stream.Write<uint32_t>(0);									// number of the disk with the start of the central directory  
	m_stream.Write<uint64_t>(m_blocks.size());						// total number of entries in the central directory on this disk  
	m_stream.Write<uint64_t>(m_blocks.size());						// total number of entries in the central directory  
	m_stream.Write<uint64_t>(centralDirectorySize);					// size of the central directory  
	m_stream.Write<uint64_t>(centralDirectoryOffset);				// offset of start of central directory with respect to the starting disk number       
																	// zip64 extensible data sector(variable size)

	// zip64 end of central directory locator.
	m_stream.Write<uint32_t>(0x07064b50);							// signature             
	m_stream.Write<uint32_t>(0);									// number of the disk with the start of the zip64 end of central directory               
	m_stream.Write<uint64_t>(endOfCentralDirectoryRecord);			// relative offset of the zip64 end of central directory record 
	m_stream.Write<uint32_t>(0);									// total number of disks         

	// Write out end of central directory.	
	m_stream.Write<uint32_t>(0x06054b50);							// End of central directory signature
	m_stream.Write<uint16_t>(0xFFFF);								// Number of this disk
	m_stream.Write<uint16_t>(0xFFFF);								// Disk where central directory starts
	m_stream.Write<uint16_t>(0xFFFF);								// Number of central directory records on this disk
	m_stream.Write<uint16_t>(0xFFFF);								// Total number of central directory records
	m_stream.Write<uint32_t>(0xFFFFFFFF);							// Size of central directory(bytes)
	m_stream.Write<uint32_t>(0xFFFFFFFF);							// Offset of start of central directory, relative to start of archive
	m_stream.Write<uint16_t>(0);									// Comment length(n)
																	// Comment

	m_stream.Close();
}

}; // namespace MicroBuild
