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

namespace MicroBuild {
namespace Platform {

// Represents a local file system path on the host platform.
class Path
{
private:
	std::string m_raw;

protected:
	void Normalize();

public:
	// Seperator of individual path segments.
	static char Seperator;

	// Constructors.
	Path();
	Path(const char* Value);
	Path(const std::string& Value);

	// Converts the path to a string.
	std::string ToString() const;

	// Concatinates the relative path to this one.
	Path operator +(const Path& Other) const;
	Path operator +(const std::string& Other) const;

	// Equality test operators.
	bool operator ==(const Path& Other) const;
	bool operator !=(const Path& Other) const;	
 	bool operator < (const Path& Other) const;
    
	// Gets the base name for the path, which is the filename without 
	// an extension.
	std::string GetBaseName() const;

	// Gets the extension of the file this path points to.
	std::string GetExtension() const;

	// Gets the filename section of this path.
	std::string GetFilename() const;

	// Gets the mount of this path (drive name etc).
	std::string GetMount() const;

	// Gets the directory this path is contained in.
	Path GetDirectory() const;

	// Returns a path with the same value but a different extension.
	Path ChangeExtension(const std::string& Value) const;

	// Returns a path with the same value but a different basename.
	Path ChangeBaseName(const std::string& Value) const;

	// Returns a path with the same value but a different filename.
	Path ChangeFilename(const std::string& Value) const;

	// Returns a path with the same value but a different mount.
	Path ChangeMount(const std::string& Value) const;

	// Returns a path with the same value but a different directory.
	Path ChangeDirectory(const Path& Value) const;

	// Appends a fragment to the path and returns it. This is a faster
	// way of doing concatanation. 
	Path AppendFragment(const std::string& Value, bool bAddDeliminator = false) const;

	// Returns a list of filenames of the files in the directory 
	// this path points to.
	std::vector<std::string> GetFiles() const;

	// Returns a list of filenames of the directories in the 
	// directory this path points to.
	std::vector<std::string> GetDirectories() const;

	// Returns a list of fragments that make up this path.
	std::vector<std::string> GetFragments() const;

	// Returns true if this path is empty.
	bool IsEmpty() const;

	// Returns true if this path points to a file.
	bool IsFile() const;

	// Returns true if this path points to a directory.
	bool IsDirectory() const;

	// Returns true if this path is relative.
	bool IsRelative() const;

	// Returns true if this path is absolute.
	bool IsAbsolute() const;

	// Returns true if this path points to a root mount.
	bool IsRoot() const;

	// Returns true if the extension of this path looks like a source file.
	bool IsSourceFile() const;

	// Returns true if the extension of this path looks like a compiled resource file.
	bool IsResourceFile() const;

	// Returns true if the extension of this path looks like an include file.
	bool IsIncludeFile() const;

	// Returns true if the extension of this path looks like a xaml file.
	bool IsXamlFile() const;

	// Returns true if the extension of this path looks like a image file.
	bool IsImageFile() const;

	// Returns true if the extension of this path looks like a c file.
	bool IsCFile() const;

	// Returns true if the extension of this path looks like a objc file.
	bool IsObjCFile() const;

	// Returns true if the extension of this path looks like a objc file.
	bool IsObjCppFile() const;

	// Returns true if the extension of this path looks like a c++ file.
	bool IsCppFile() const;

	// Returns true if the extension of this path looks like a csharp file.
	bool IsCSharpFile() const;

	// Creates this path as a directory.
	bool CreateAsDirectory() const;

	// Creates this path as a file.
	bool CreateAsFile() const;

	// Returns true if this path exists.
	bool Exists() const;

	// Copies the file or directory this path points to, to another 
	// directory or file.
	bool Copy(const Path& Destination) const;

	// Deletes this path recursively.
	bool Delete() const;

	// Gets the time this path was last modified.
	std::time_t GetModifiedTime() const;

	// Creates a path that represents a relative reference from this path
	// to the given destination path.
	Path RelativeTo(const Path& Destination) const;

	// Attempts to find all file or directories that match the given filter.
	// Filters can use * for wildcards, and ** for recursive wildcards.
	// eg. 
	//	./MyFolder/*.ini 
	//	./MyFolder/**/Project.ini
	//	./**.ini
	// Input path should be absolute.
	static std::vector<Path> MatchFilter(const Path& path);

	// Performs th same matching logic as MatchFilter to see if the given
	// path would match the given filter.
	// If Matched parameter is set it is set to the path segments matched
	// for wildcards.
	bool Matches(const Path& other, Path* Matched = nullptr);

	// Gets the common path that a list of paths share, returns false if
	// all the paths do not share a common path.
	static bool GetCommonPath(std::vector<Path>& paths, Path& result);

	// Gets the current working directory, should be used only for expanding
	// command line input.
	static Path GetWorkingDirectory();

	// Sets the current working directory.
	static void SetWorkingDirectory(const Path& other);	

	// Gets the path of the executable.
	static Path GetExecutablePath();

	// Sets the current executable directory.
	static void SetExecutablePath(const Path& other);

	// Finds the first file with the given filename in the environment path variable and any
	// additional directories given.
	static bool FindFile(const std::string& filename, Path& result);
	static bool FindFile(const std::string& filename, Path& result, std::vector<Platform::Path> additionalDirs);

	// Returns the part of the path following the common path that is
	// has been obtained from GetCommonPath.
	Path GetUncommonPath(Path& commonPath);

};

}; // namespace Platform
}; // namespace MicroBuild
