/*
	This file is part of Hikari, a library that allows developers
	to use Flash in their Ogre3D applications.

	Copyright (C) 2008 Adam J. Simmons

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __HikariPlatform_H__
#define __HikariPlatform_H__

// Modern-build fix (not a team patch -- see docs/porting/hikari-gui.md):
// hu_hikari (modern/CMakeLists.txt) builds Hikari as a static library
// (matching the convention of every other ported project in this tree)
// rather than the original Hikari.dll, so _HikariExport must not resolve
// to dllexport/dllimport at all in that configuration. hu_hikari defines
// HIKARI_STATIC PUBLIC so every consumer (including this header) sees it.
#if defined(HIKARI_STATIC)
#	define _HikariExport
#elif defined(HIKARI_NONCLIENT_BUILD)
#	define _HikariExport __declspec( dllexport )
#else
#	define _HikariExport __declspec( dllimport )
#endif

#endif