/* copyright 2012 - 2013 Sascha Kruse and contributors (see LICENSE for licensing information) */

#include <assert.h>
#include <glib.h>
#include <stdbool.h>

bool parse_bool(const char *boolstr, bool *ret)
{
        assert(boolstr);
        assert(ret);

        switch (boolstr[0]) {
        case 'y':
        case 'Y':
        case 't':
        case 'T':
        case '1':
                *ret = true;
                return true;
        case 'n':
        case 'N':
        case 'f':
        case 'F':
        case '0':
                *ret = false;
                return true;
        default:
                return false;
        }
}

/* vim: set tabstop=8 shiftwidth=8 expandtab textwidth=0: */
