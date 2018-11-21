/* copyright 2013 Sascha Kruse and contributors (see LICENSE for licensing information) */

#ifndef CMD_DBUS_CLIENT_H
#define CMD_DBUS_CLIENT_H

#include <gio/gio.h>
#include <glib.h>

#define PROP_IFAC "org.freedesktop.DBus.Properties"

#define FDN_PATH "/org/freedesktop/Notifications"
#define FDN_IFAC "org.freedesktop.Notifications"
#define FDN_NAME "org.freedesktop.Notifications"

#define DUNST_PATH "/org/freedesktop/Notifications"
#define DUNST_IFAC "org.dunstproject.cmd0"
#define DUNST_NAME "org.freedesktop.Notifications"

/** Retrieve a property from the current dunst instance
 *
 * @param property The porperty string of the given dunst property
 * @param error (nullable) The error message, if wanted
 * @returns A GVariant with the values of the specified property.
 */
GVariant* dbus_client_get_property(const char *property, GError **error);

#endif
/* vim: set tabstop=8 shiftwidth=8 expandtab textwidth=0: */
