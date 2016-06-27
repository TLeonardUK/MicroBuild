--[[
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
--]]

-- Figure out the root directory for microbuild. This makes assumptions
-- on directory layout, so don't fiddle :)
MBRoot = os.realpath(os.getcwd() .. "/../../../")
print("MicroBuild Root: " .. MBRoot)

include("Workspace.lua")
include("Project.lua");