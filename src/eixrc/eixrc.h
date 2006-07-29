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

#ifndef __EIXRC_H__
#define __EIXRC_H__

#include <eixTk/exceptions.h>
#include <varsreader.h>

#include <vector>
#include <string>
#include <portage/keywords.h>

#define EIX_USERRC   "/.eixrc"

#ifndef SYSCONFDIR
#define SYSCONFDIR "/etc"
#endif /* SYSCONFDIR */

#define EIX_SYSTEMRC SYSCONFDIR"/eixrc"

class EixRcOption {
	public:
		static const char STRING = 0, INTEGER = 1, BOOLEAN = 2;
		char type;
		std::string key, value, description;
		EixRcOption(char t, std::string name, std::string val, std::string desc) {
			type = t;
			key = name;
			value = val;
			description = desc;
		}
};

class EixRc : public std::map<std::string,std::string> {

	private:
		std::vector<EixRcOption> defaults;

	public:
		void read(void) {
			char *home = getenv("HOME");
			if(!(home))
				WARNING("No $HOME found in environment.");
			else
			{
				std::string eixrc(home);
				eixrc.append(EIX_USERRC);

				VarsReader rc(VarsReader::SUBST_VARS
						|VarsReader::INTO_MAP);
				rc.useMap(this);
				rc.read(EIX_SYSTEMRC);
				rc.read(eixrc.c_str());

				// look for stuff from ENV
				for(unsigned int i = 0;
					i<defaults.size();
					++i)
				{
					char *val = getenv(defaults[i].key.c_str());
					if(val != NULL)
					{
						(*this)[defaults[i].key] = std::string(val);
					}
				}
			}
		}

		void clear() {
			defaults.clear();
			((std::map<std::string,std::string>*) this)->clear();
		}

		void addDefault(EixRcOption option) {
			defaults.push_back(option);
			(*this)[option.key] = option.value;
		}

		bool getBool(const char *key) {
			return ! strcasecmp((*this)[key].c_str(),"true");
		}

		void getRedundantFlags(const char *key,
			Keywords::Redundant type,
			Keywords::Redundant &red,
			Keywords::Redundant &all,
			Keywords::Redundant &spc,
			Keywords::Redundant &ins)
		{
			const char *a=(*this)[key].c_str();
			if((strcasecmp(a, "no") == 0) ||
			   (strcasecmp(a, "false") == 0))
			{
				red &= ~type;
			}
			else if(strcasecmp(a, "some") == 0)
			{
				red |= type;
				all &= ~type;
				spc &= ~type;
			}
			else if(strcasecmp(a, "some-installed") == 0)
			{
				red |= type;
				all &= ~type;
				spc |= type;
				ins |= type;
			}
			else if(strcasecmp(a, "some-uninstalled") == 0)
			{
				red |= type;
				all &= ~type;
				spc |= type;
				ins &= ~type;
			}
			else if(strcasecmp(a, "all") == 0)
			{
				red |= type;
				all |= type;
				spc &= ~type;
			}
			else if(strcasecmp(a, "all-installed") == 0)
			{
				red |= type;
				all |= type;
				spc |= type;
				ins |= type;
			}
			else if(strcasecmp(a, "all-uninstalled") == 0)
			{
				red |= type;
				all |= type;
				spc |= type;
				ins &= ~type;
			}
			else
			{
				WARNING("Variable %s has unknown value %s.\n"
					"Assuming value all-installed",	key, a);
				red |= type;
				all |= type;
				spc |= type;
				ins |= ~type;
			}
		}

		int getInteger(const char *key) {
			return atoi((*this)[key].c_str());
		}

		void dumpDefaults(FILE *s) {
			for(unsigned int i = 0;
				i<defaults.size();
				++i)
			{
				const char *typestring = "UNKNOWN";
				switch(defaults[i].type) {
					case EixRcOption::BOOLEAN: typestring = "BOOLEAN";
								  break;
					case EixRcOption::STRING: typestring = "STRING";
								  break;
					case EixRcOption::INTEGER: typestring = "INTEGER";
								  break;
				}

				fprintf(s,
						"# %s\n"
						"# %s\n"
						"%s='%s'\n\n",
						typestring,
						defaults[i].description.c_str(),
						defaults[i].key.c_str(),
						defaults[i].value.c_str());
			}
		}
};
#endif /* __EIXRC_H__ */
