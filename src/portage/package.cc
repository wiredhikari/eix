/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *     Martin V�th <vaeth@mathematik.uni-wuerzburg.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "package.h"

#include <portage/version.h>

Package::~Package()
{
	delete_and_clear();
}

const Package::Duplicates
	Package::DUP_NONE     = 0x00,
	Package::DUP_SOME     = 0x01,
	Package::DUP_OVERLAYS = 0x03;

const Package::Slots
	Package::SLOTS_NONE   = 0x00,
	Package::SLOTS_EXIST  = 0x01,
	Package::SLOTS_MANY   = 0x03;

/** Check if a package has duplicated versions. */
void Package::checkDuplicates(Version *version)
{
	if(have_duplicate_versions == DUP_OVERLAYS)
		return;
	bool no_overlay = !(version->overlay_key);
	if(no_overlay && (have_duplicate_versions == DUP_SOME))
		return;
	for(iterator i = begin(); i != end(); ++i)
	{
		if(dynamic_cast<BasicVersion&>(**i) == dynamic_cast<BasicVersion&>(*version))
		{
			if(no_overlay)
			{
				have_duplicate_versions = DUP_SOME;
				return;
			}
			if(i->overlay_key)
			{
				have_duplicate_versions = DUP_OVERLAYS;
				return;
			}
			have_duplicate_versions = DUP_SOME;
		}
	}
}

/** Finishes addVersionStart() after the remaining data
    have been filled */
void Package::addVersionFinalize(Version *version)
{
	Version::Overlay key = version->overlay_key;

	if(version->slot == "0")
		version->slot = "";

	/* This guarantees that we pushed our first version */
	if(size() != 1) {
		if(largest_overlay != key)
		{
			have_same_overlay_key = false;
			if(largest_overlay && key)
				at_least_two_overlays = true;
			if(largest_overlay < key)
				largest_overlay = key;
		}
		if((have_slots & SLOTS_MANY) != SLOTS_MANY)
		{
			if(common_slot != version->slot)
				have_slots |= SLOTS_MANY;
		}
		if(is_system_package) {
			is_system_package = version->isSystem();
		}
	}
	else {
		largest_overlay   = key;
		if( (version->slot).length() )
		{
			have_slots  = SLOTS_EXIST;
			common_slot = version->slot;
		}
		is_system_package = version->isSystem();
	}
}

void
Package::sortedPushBack(Version *v)
{
	for(iterator i = begin(); i != end(); ++i)
	{
		if(*v < **i)
		{
			insert(i, v);
			return;
		}
	}
	push_back(v);
}

Version *
Package::best() const
{
	Version *ret = NULL;
	for(const_reverse_iterator ri = rbegin();
		ri != rend();
		++ri)
	{
		if(ri->isStable() && !ri->isHardMasked())
		{
			ret = *ri;
			break;
		}
	}
	return ret;
}

void Package::deepcopy(const Package &p)
{
	*this=p;
	clear();
	for(Package::const_iterator it=p.begin(); it != p.end(); it++)
	{
		Version *v=new Version;
		*v=(**it);
		this->push_back(v);
	}
}
