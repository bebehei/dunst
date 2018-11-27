/* copyright 2012 - 2013 Sascha Kruse and contributors (see LICENSE for licensing information) */

#include "dbus-client.h"

#include <gio/gio.h>
#include <glib.h>
#include <stdbool.h>
#include <stdlib.h>

#include "utils.h"

static GDBusConnection *conn;
static GDBusProxy *property_proxy;

/**
 * Asserts that the current dunst server process is actually controllable.
 * In case of incompatible dunst version, it Won't return and will die with
 * an appropriate error message.
 */
static void dbus_client_assert_compatible_server(void)
{
        static bool dunst_compatible_server = false;
        if (dunst_compatible_server)
                return;

        GVariant *output_introspect;
        GError *error = NULL;

        output_introspect = g_dbus_connection_call_sync(
                                        conn,
                                        DUNST_NAME,
                                        DUNST_PATH,
                                        "org.freedesktop.DBus.Introspectable",
                                        "Introspect",
                                        NULL,
                                        NULL,
                                        G_DBUS_CALL_FLAGS_NONE,
                                        -1,
                                        NULL,
                                        &error);

        if (error) {
                DIE("Interface not available: %s", error->message);
                g_clear_error(&error);
        }

        GDBusNodeInfo *node;
        GDBusInterfaceInfo *ifac;
        char *introspection_data;

        g_variant_get(output_introspect, "(s)", &introspection_data);
        node = g_dbus_node_info_new_for_xml(introspection_data, NULL);
        ifac = g_dbus_node_info_lookup_interface(node, DUNST_IFAC);

        if (ifac)
                dunst_compatible_server = true;
        else
                DIE("There is either no dunst running or the dunst version is not controllable.");

        g_dbus_interface_info_unref(ifac);
        g_dbus_node_info_unref(node);
}

/**
 * Assert that a dbus connection is available.
 *
 * If unsuccessful to assert, it won't return.
 */
static void dbus_client_assert_conn(void)
{
        if (conn)
                return;

        GError *error = NULL;
        conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
        if (error)
                DIE("Cannot establish DBus connection: %s", error->message);

        dbus_client_assert_compatible_server();
}

/**
 * Assert that the GDBusProxy to the Properties interface is granted.
 *
 * If unsuccessful to assert, it won't return.
 */
void dbus_client_assert_proxy(void)
{
        if (property_proxy)
                return;

        dbus_client_assert_conn();

        GError *error = NULL;
        property_proxy = g_dbus_proxy_new_sync(
                                     conn,
                                     G_DBUS_PROXY_FLAGS_NONE,
                                     NULL, /* info */
                                     DUNST_NAME,
                                     DUNST_PATH,
                                     PROP_IFAC,
                                     NULL, /* cancelable */
                                     &error);

        //TODO: Write, that the current instance may not be compatible
        if (error)
                DIE("Cannot connect to Properties: %s", error->message);
}

GVariant* dbus_client_get_property(const char *property, GError **error)
{
        dbus_client_assert_proxy();

        GVariant *info = g_dbus_proxy_call_sync(
                                     property_proxy,
                                     PROP_IFAC ".Get",
                                     g_variant_new("(ss)", DUNST_IFAC, property),
                                     G_DBUS_CALL_FLAGS_NONE,
                                     -1,
                                     NULL,
                                     error);
        if (error && *error)
                return NULL;

        GVariant *ret;
        g_variant_get(info, "(v)", &ret);

        return ret;
}

void dbus_client_set_property(const char *property, GVariant *value, GError **error)
{
        dbus_client_assert_proxy();

        GVariant *info = g_dbus_proxy_call_sync(
                                     property_proxy,
                                     PROP_IFAC ".Set",
                                     g_variant_new("(ssv)", DUNST_IFAC, property, value),
                                     G_DBUS_CALL_FLAGS_NONE,
                                     -1,
                                     NULL,
                                     error);

        if (info)
                g_variant_unref(info);
}

void dbus_client_listen(const char *signal, GCallback cb)
{
        dbus_client_assert_proxy();

        GError *error = NULL;
        GDBusProxy *p = g_dbus_proxy_new_sync(
                                     conn,
                                     G_DBUS_PROXY_FLAGS_NONE,
                                     NULL, /* info */
                                     DUNST_NAME,
                                     DUNST_PATH,
                                     DUNST_IFAC,
                                     NULL, /* cancelable */
                                     &error);

        g_signal_connect(p, signal, cb, NULL);
}

/* vim: set tabstop=8 shiftwidth=8 expandtab textwidth=0: */
