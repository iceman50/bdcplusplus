/*
 * Copyright (C) 2001-2023 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DCPLUSPLUS_DCPP_FORMAT_H_
#define DCPLUSPLUS_DCPP_FORMAT_H_

#include <libintl.h>

// libintl's "#define snprintf libintl_snprintf" does not play nicely with
// Boost using std::snprintf. https://github.com/boostorg/system/issues/32
// describes this issue in the context of #define snprintf _snprintf which
// creates analogous problems. libintl's snprintf isn't part of localizing
// per se, so allow Boost to use std::snprintf(...), unobstructed by macro
// definitions.
//
// https://www.c-plusplus.net/forum/topic/231492/bin-am-verzweifeln-error-libintl_snprintf-is-not-a-member-of-std
// notes, for example: "Für mich sieht das sehr stark danach aus, daß einer
// deiner anderen Header sowas wie #define snprintf libintl_snprintf macht.
// Und sowas macht man natürlich einfach nicht." But libintl does, exactly,
// that.
#undef snprintf

#include <boost/format.hpp>

#ifdef BUILDING_DCPP

#define PACKAGE "libdcpp"
#define LOCALEDIR dcpp::Util::getPath(Util::PATH_LOCALE).c_str()
#define _(String) dgettext(PACKAGE, String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
#define F_(String) dcpp::dcpp_fmt(dgettext(PACKAGE, String))
#define FN_(String1,String2, N) dcpp::dcpp_fmt(dngettext(PACKAGE, String1, String2, N))

#endif

namespace dcpp {

template<typename T>
boost::basic_format<T> dcpp_fmt(const std::basic_string<T>& t) {
	boost::basic_format<T> fmt;
	fmt.exceptions(boost::io::no_error_bits);
	fmt.parse(t);
	return fmt;
}

template<typename T>
boost::basic_format<T> dcpp_fmt(const T* t) {
	return dcpp_fmt(std::basic_string<T>(t));
}

}

using boost::str;

#endif /* DCPLUSPLUS_DCPP_FORMAT_H_ */
