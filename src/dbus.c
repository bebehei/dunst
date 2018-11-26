/* copyright 2013 Sascha Kruse and contributors (see LICENSE for licensing information) */
#include "dbus.h"

#include <gio/gio.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "dunst.h"
#include "log.h"
#include "notification.h"
#include "queues.h"
#include "settings.h"
#include "utils.h"

#define FDN_PATH "/org/freedesktop/Notifications"
#define FDN_IFAC "org.freedesktop.Notifications"
#define FDN_NAME "org.freedesktop.Notifications"

#define DUNST_PATH "/org/freedesktop/Notifications"
#define DUNST_IFAC "org.dunstproject.cmd0"
#define DUNST_NAME "org.freedesktop.Notifications"

enum dbus_method {
        DBUS_METHOD_INVALID = 0,
        DBUS_METHOD_MIN = 1,
        DBUS_METHOD_FDO_CAPS = 1,
        DBUS_METHOD_FDO_NOTIFY = 2,
        DBUS_METHOD_FDO_CLOSE = 3,
        DBUS_METHOD_FDO_SERVINFO = 4,
        DBUS_PROPERTY_SET = 5,
        DBUS_PROPERTY_GET = 6,
        DBUS_PROPERTY_GETALL = 7,
        DBUS_METHOD_MAX = 8,
};

GDBusConnection *dbus_conn;

static GDBusNodeInfo *introspection_data = NULL;

static const char *introspection_xml =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<node name=\""FDN_PATH"\">"
    "    <interface name=\""FDN_IFAC"\">"

    "        <method name=\"GetCapabilities\">"
    "            <arg direction=\"out\" name=\"capabilities\"    type=\"as\"/>"
    "        </method>"

    "        <method name=\"Notify\">"
    "            <arg direction=\"in\"  name=\"app_name\"        type=\"s\"/>"
    "            <arg direction=\"in\"  name=\"replaces_id\"     type=\"u\"/>"
    "            <arg direction=\"in\"  name=\"app_icon\"        type=\"s\"/>"
    "            <arg direction=\"in\"  name=\"summary\"         type=\"s\"/>"
    "            <arg direction=\"in\"  name=\"body\"            type=\"s\"/>"
    "            <arg direction=\"in\"  name=\"actions\"         type=\"as\"/>"
    "            <arg direction=\"in\"  name=\"hints\"           type=\"a{sv}\"/>"
    "            <arg direction=\"in\"  name=\"expire_timeout\"  type=\"i\"/>"
    "            <arg direction=\"out\" name=\"id\"              type=\"u\"/>"
    "        </method>"

    "        <method name=\"CloseNotification\">"
    "            <arg direction=\"in\"  name=\"id\"              type=\"u\"/>"
    "        </method>"

    "        <method name=\"GetServerInformation\">"
    "            <arg direction=\"out\" name=\"name\"            type=\"s\"/>"
    "            <arg direction=\"out\" name=\"vendor\"          type=\"s\"/>"
    "            <arg direction=\"out\" name=\"version\"         type=\"s\"/>"
    "            <arg direction=\"out\" name=\"spec_version\"    type=\"s\"/>"
    "        </method>"

    "        <signal name=\"NotificationClosed\">"
    "            <arg name=\"id\"         type=\"u\"/>"
    "            <arg name=\"reason\"     type=\"u\"/>"
    "        </signal>"

    "        <signal name=\"ActionInvoked\">"
    "            <arg name=\"id\"         type=\"u\"/>"
    "            <arg name=\"action_key\" type=\"s\"/>"
    "        </signal>"
    "   </interface>"
    "    <interface name=\""DUNST_IFAC"\">"

    "        <property name=\"running\" type=\"b\" access=\"readwrite\">"
    "            <annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\"/>"
    "        </property>"

    "   </interface>"
    "</node>";

static const char *stack_tag_hints[] = {
        "synchronous",
        "private-synchronous",
        "x-canonical-private-synchronous",
        "x-dunst-stack-tag"
};

static void on_get_capabilities(GDBusConnection *connection,
                                const gchar *sender,
                                const GVariant *parameters,
                                GDBusMethodInvocation *invocation);
static void on_notify(GDBusConnection *connection,
                      const gchar *sender,
                      GVariant *parameters,
                      GDBusMethodInvocation *invocation);
static void on_close_notification(GDBusConnection *connection,
                                  const gchar *sender,
                                  GVariant *parameters,
                                  GDBusMethodInvocation *invocation);
static void on_get_server_information(GDBusConnection *connection,
                                      const gchar *sender,
                                      const GVariant *parameters,
                                      GDBusMethodInvocation *invocation);
static struct raw_image *get_raw_image_from_data_hint(GVariant *icon_data);

enum dbus_method dbus_select_method(const gchar *interface_name, const gchar *method_name)
{
        if (STR_EQ(interface_name, "org.freedesktop.DBus.Properties")) {
                if (STR_EQ(method_name, "Set")) {
                        return DBUS_PROPERTY_SET;
                } else if (STR_EQ(method_name, "Get")) {
                        return DBUS_PROPERTY_GET;
                } else if (STR_EQ(method_name, "GetAll")) {
                        return DBUS_PROPERTY_GETALL;
                } else {
                        return DBUS_METHOD_INVALID;
                }
        } else if (STR_EQ(interface_name, DUNST_IFAC)) {
                return DBUS_METHOD_INVALID;
        } else if (STR_EQ(interface_name, FDN_IFAC)) {
                if (STR_EQ(method_name, "GetCapabilities")) {
                        return DBUS_METHOD_FDO_CAPS;
                } else if (STR_EQ(method_name, "Notify")) {
                        return DBUS_METHOD_FDO_NOTIFY;
                } else if (STR_EQ(method_name, "CloseNotification")) {
                        return DBUS_METHOD_FDO_CLOSE;
                } else if (STR_EQ(method_name, "GetServerInformation")) {
                        return DBUS_METHOD_FDO_SERVINFO;
                } else {
                        return DBUS_METHOD_INVALID;
                }
        } else {
                return DBUS_METHOD_INVALID;
        }
}

void handle_method_call(GDBusConnection *connection,
                        const gchar *sender,
                        const gchar *object_path,
                        const gchar *interface_name,
                        const gchar *method_name,
                        GVariant *parameters,
                        GDBusMethodInvocation *invocation,
                        gpointer user_data)
{
        switch (dbus_select_method(interface_name, method_name)) {
        case DBUS_METHOD_FDO_CAPS:
                on_get_capabilities(connection, sender, parameters, invocation);
                break;

        case DBUS_METHOD_FDO_NOTIFY:
                on_notify(connection, sender, parameters, invocation);
                break;

        case DBUS_METHOD_FDO_CLOSE:
                on_close_notification(connection, sender, parameters, invocation);
                break;

        case DBUS_METHOD_FDO_SERVINFO:
                on_get_server_information(connection, sender, parameters, invocation);
                break;

        default:
                LOG_M("Unknown method '%s' on interface '%s' (sender: '%s') received.",
                      method_name,
                      interface_name,
                      sender);
                break;
        }
}

static void on_get_capabilities(GDBusConnection *connection,
                                const gchar *sender,
                                const GVariant *parameters,
                                GDBusMethodInvocation *invocation)
{
        GVariantBuilder *builder;
        GVariant *value;

        builder = g_variant_builder_new(G_VARIANT_TYPE("as"));
        g_variant_builder_add(builder, "s", "actions");
        g_variant_builder_add(builder, "s", "body");
        g_variant_builder_add(builder, "s", "body-hyperlinks");

        for (int i = 0; i < sizeof(stack_tag_hints)/sizeof(*stack_tag_hints); ++i)
                g_variant_builder_add(builder, "s", stack_tag_hints[i]);

        if (settings.markup != MARKUP_NO)
                g_variant_builder_add(builder, "s", "body-markup");

        value = g_variant_new("(as)", builder);
        g_clear_pointer(&builder, g_variant_builder_unref);
        g_dbus_method_invocation_return_value(invocation, value);

        g_dbus_connection_flush(connection, NULL, NULL, NULL);
}

static struct notification *dbus_message_to_notification(const gchar *sender, GVariant *parameters)
{

        struct notification *n = notification_create();

        n->actions = g_malloc0(sizeof(struct actions));
        n->dbus_client = g_strdup(sender);
        n->dbus_valid = true;

        {
                GVariantIter *iter = g_variant_iter_new(parameters);
                GVariant *content;
                GVariant *dict_value;
                int idx = 0;
                while ((content = g_variant_iter_next_value(iter))) {

                        switch (idx) {
                        case 0:
                                if (g_variant_is_of_type(content, G_VARIANT_TYPE_STRING))
                                        n->appname = g_variant_dup_string(content, NULL);
                                break;
                        case 1:
                                if (g_variant_is_of_type(content, G_VARIANT_TYPE_UINT32))
                                        n->id = g_variant_get_uint32(content);
                                break;
                        case 2:
                                if (g_variant_is_of_type(content, G_VARIANT_TYPE_STRING))
                                        n->icon = g_variant_dup_string(content, NULL);
                                break;
                        case 3:
                                if (g_variant_is_of_type(content, G_VARIANT_TYPE_STRING))
                                        n->summary = g_variant_dup_string(content, NULL);
                                break;
                        case 4:
                                if (g_variant_is_of_type(content, G_VARIANT_TYPE_STRING))
                                        n->body = g_variant_dup_string(content, NULL);
                                break;
                        case 5:
                                if (g_variant_is_of_type(content, G_VARIANT_TYPE_STRING_ARRAY))
                                        n->actions->actions = g_variant_dup_strv(content, &(n->actions->count));
                                break;
                        case 6:
                                if (g_variant_is_of_type(content, G_VARIANT_TYPE_DICTIONARY)) {

                                        dict_value = g_variant_lookup_value(content, "urgency", G_VARIANT_TYPE_BYTE);
                                        if (dict_value) {
                                                n->urgency = g_variant_get_byte(dict_value);
                                                g_variant_unref(dict_value);
                                        }

                                        dict_value = g_variant_lookup_value(content, "fgcolor", G_VARIANT_TYPE_STRING);
                                        if (dict_value) {
                                                n->colors[ColFG] = g_variant_dup_string(dict_value, NULL);
                                                g_variant_unref(dict_value);
                                        }

                                        dict_value = g_variant_lookup_value(content, "bgcolor", G_VARIANT_TYPE_STRING);
                                        if (dict_value) {
                                                n->colors[ColBG] = g_variant_dup_string(dict_value, NULL);
                                                g_variant_unref(dict_value);
                                        }

                                        dict_value = g_variant_lookup_value(content, "frcolor", G_VARIANT_TYPE_STRING);
                                        if (dict_value) {
                                                n->colors[ColFrame] = g_variant_dup_string(dict_value, NULL);
                                                g_variant_unref(dict_value);
                                        }

                                        dict_value = g_variant_lookup_value(content, "category", G_VARIANT_TYPE_STRING);
                                        if (dict_value) {
                                                n->category = g_variant_dup_string(dict_value, NULL);
                                                g_variant_unref(dict_value);
                                        }

                                        dict_value = g_variant_lookup_value(content, "image-path", G_VARIANT_TYPE_STRING);
                                        if (dict_value) {
                                                g_free(n->icon);
                                                n->icon = g_variant_dup_string(dict_value, NULL);
                                                g_variant_unref(dict_value);
                                        }

                                        dict_value = g_variant_lookup_value(content, "image-data", G_VARIANT_TYPE("(iiibiiay)"));
                                        if (!dict_value)
                                                dict_value = g_variant_lookup_value(content, "image_data", G_VARIANT_TYPE("(iiibiiay)"));
                                        if (!dict_value)
                                                dict_value = g_variant_lookup_value(content, "icon_data", G_VARIANT_TYPE("(iiibiiay)"));
                                        if (dict_value) {
                                                n->raw_icon = get_raw_image_from_data_hint(dict_value);
                                                g_variant_unref(dict_value);
                                        }

                                        /* Check for transient hints
                                         *
                                         * According to the spec, the transient hint should be boolean.
                                         * But notify-send does not support hints of type 'boolean'.
                                         * So let's check for int and boolean until notify-send is fixed.
                                         */
                                        if((dict_value = g_variant_lookup_value(content, "transient", G_VARIANT_TYPE_BOOLEAN))) {
                                                n->transient = g_variant_get_boolean(dict_value);
                                                g_variant_unref(dict_value);
                                        } else if((dict_value = g_variant_lookup_value(content, "transient", G_VARIANT_TYPE_UINT32))) {
                                                n->transient = g_variant_get_uint32(dict_value) > 0;
                                                g_variant_unref(dict_value);
                                        } else if((dict_value = g_variant_lookup_value(content, "transient", G_VARIANT_TYPE_INT32))) {
                                                n->transient = g_variant_get_int32(dict_value) > 0;
                                                g_variant_unref(dict_value);
                                        }

                                        if((dict_value = g_variant_lookup_value(content, "value", G_VARIANT_TYPE_INT32))) {
                                                n->progress = g_variant_get_int32(dict_value);
                                                g_variant_unref(dict_value);
                                        } else if((dict_value = g_variant_lookup_value(content, "value", G_VARIANT_TYPE_UINT32))) {
                                                n->progress = g_variant_get_uint32(dict_value);
                                                g_variant_unref(dict_value);
                                        }

                                        /* Check for hints that define the stack_tag
                                         *
                                         * Only accept to first one we find.
                                         */
                                        for (int i = 0; i < sizeof(stack_tag_hints)/sizeof(*stack_tag_hints); ++i) {
                                                dict_value = g_variant_lookup_value(content, stack_tag_hints[i], G_VARIANT_TYPE_STRING);
                                                if (dict_value) {
                                                        n->stack_tag = g_variant_dup_string(dict_value, NULL);
                                                        g_variant_unref(dict_value);
                                                        break;
                                                }
                                        }

                                }
                                break;
                        case 7:
                                if (g_variant_is_of_type(content, G_VARIANT_TYPE_INT32))
                                        n->timeout = g_variant_get_int32(content) * 1000;
                                break;
                        }
                        g_variant_unref(content);
                        idx++;
                }

                g_variant_iter_free(iter);
        }

        if (n->actions->count < 1)
                g_clear_pointer(&n->actions, actions_free);

        notification_init(n);
        return n;
}

static void on_notify(GDBusConnection *connection,
                      const gchar *sender,
                      GVariant *parameters,
                      GDBusMethodInvocation *invocation)
{
        struct notification *n = dbus_message_to_notification(sender, parameters);
        int id = queues_notification_insert(n);

        GVariant *reply = g_variant_new("(u)", id);
        g_dbus_method_invocation_return_value(invocation, reply);
        g_dbus_connection_flush(connection, NULL, NULL, NULL);

        // The message got discarded
        if (id == 0) {
                signal_notification_closed(n, 2);
                notification_unref(n);
        }

        wake_up();
}

static void on_close_notification(GDBusConnection *connection,
                                  const gchar *sender,
                                  GVariant *parameters,
                                  GDBusMethodInvocation *invocation)
{
        guint32 id;
        g_variant_get(parameters, "(u)", &id);
        queues_notification_close_id(id, REASON_SIG);
        wake_up();
        g_dbus_method_invocation_return_value(invocation, NULL);
        g_dbus_connection_flush(connection, NULL, NULL, NULL);
}

static void on_get_server_information(GDBusConnection *connection,
                                      const gchar *sender,
                                      const GVariant *parameters,
                                      GDBusMethodInvocation *invocation)
{
        GVariant *value;

        value = g_variant_new("(ssss)", "dunst", "knopwob", VERSION, "1.2");
        g_dbus_method_invocation_return_value(invocation, value);

        g_dbus_connection_flush(connection, NULL, NULL, NULL);
}

void signal_notification_closed(struct notification *n, enum reason reason)
{
        if (!n->dbus_valid) {
                LOG_W("Closing notification '%s' not supported. "
                      "Notification already closed.", n->summary);
                return;
        }

        if (reason < REASON_MIN || REASON_MAX < reason) {
                LOG_W("Closing notification with reason '%d' not supported. "
                      "Closing it with reason '%d'.", reason, REASON_UNDEF);
                reason = REASON_UNDEF;
        }

        if (!dbus_conn) {
                LOG_E("Unable to close notification: No DBus connection.");
        }

        GVariant *body = g_variant_new("(uu)", n->id, reason);
        GError *err = NULL;

        g_dbus_connection_emit_signal(dbus_conn,
                                      n->dbus_client,
                                      FDN_PATH,
                                      FDN_IFAC,
                                      "NotificationClosed",
                                      body,
                                      &err);

        n->dbus_valid = false;

        if (err) {
                LOG_W("Unable to close notification: %s", err->message);
                g_error_free(err);
        }

}

void signal_action_invoked(const struct notification *n, const char *identifier)
{
        if (!n->dbus_valid) {
                LOG_W("Invoking action '%s' not supported. "
                      "Notification already closed.", identifier);
                return;
        }

        GVariant *body = g_variant_new("(us)", n->id, identifier);
        GError *err = NULL;

        g_dbus_connection_emit_signal(dbus_conn,
                                      n->dbus_client,
                                      FDN_PATH,
                                      FDN_IFAC,
                                      "ActionInvoked",
                                      body,
                                      &err);

        if (err) {
                LOG_W("Unable to invoke action: %s", err->message);
                g_error_free(err);
        }
}

static const GDBusInterfaceVTable interface_vtable = {
        handle_method_call
};

static void on_bus_acquired(GDBusConnection *connection,
                            const gchar *name,
                            gpointer user_data)
{
        guint registration_id;

        GError *err = NULL;

        for (GDBusInterfaceInfo **ifaces = introspection_data->interfaces;
                                 *ifaces;
                                  ifaces++) {
                registration_id = g_dbus_connection_register_object(connection,
                                                                    FDN_PATH,
                                                                    *ifaces,
                                                                    &interface_vtable,
                                                                    NULL,
                                                                    NULL,
                                                                    &err);

                if (registration_id == 0) {
                        DIE("Unable to register dbus connection interface '%s': %s", (*ifaces)->name, err->message);
                }
        }
}

static void on_name_acquired(GDBusConnection *connection,
                             const gchar *name,
                             gpointer user_data)
{
        dbus_conn = connection;
}

/**
 * Get the PID of the current process, which acquired FDN DBus Name.
 *
 * If name or vendor specified, the name and vendor
 * will get additionally get via the FDN GetServerInformation method
 *
 * @param connection The dbus connection
 * @param pid The place to report the PID to
 * @param name The place to report the name to, if not required set to NULL
 * @param vendor The place to report the vendor to, if not required set to NULL
 *
 * @returns `true` on success, otherwise `false`
 */
static bool dbus_get_fdn_daemon_info(GDBusConnection  *connection,
                                    int   *pid,
                                    char **name,
                                    char **vendor)
{
        g_return_val_if_fail(pid, false);
        g_return_val_if_fail(connection, false);

        char *owner = NULL;
        GError *error = NULL;

        GDBusProxy *proxy_fdn;
        GDBusProxy *proxy_dbus;

        proxy_fdn = g_dbus_proxy_new_sync(
                                     connection,
                                     /* do not trigger a start of the notification daemon */
                                     G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                                     NULL, /* info */
                                     FDN_NAME,
                                     FDN_PATH,
                                     FDN_IFAC,
                                     NULL, /* cancelable */
                                     &error);

        if (error) {
                g_error_free(error);
                return false;
        }

        GVariant *daemoninfo;
        if (name || vendor) {
                daemoninfo = g_dbus_proxy_call_sync(
                                     proxy_fdn,
                                     FDN_IFAC ".GetServerInformation",
                                     NULL,
                                     G_DBUS_CALL_FLAGS_NONE,
                                     /* It's not worth to wait for the info
                                      * longer than half a second when dying */
                                     500,
                                     NULL, /* cancelable */
                                     &error);
        }

        if (error) {
                /* Ignore the error, we may still be able to retrieve the PID */
                g_clear_pointer(&error, g_error_free);
        } else {
                g_variant_get(daemoninfo, "(ssss)", name, vendor, NULL, NULL);
        }

        owner = g_dbus_proxy_get_name_owner(proxy_fdn);

        proxy_dbus = g_dbus_proxy_new_sync(
                                     connection,
                                     G_DBUS_PROXY_FLAGS_NONE,
                                     NULL, /* info */
                                     "org.freedesktop.DBus",
                                     "/org/freedesktop/DBus",
                                     "org.freedesktop.DBus",
                                     NULL, /* cancelable */
                                     &error);

        if (error) {
                g_error_free(error);
                return false;
        }

        GVariant *pidinfo = g_dbus_proxy_call_sync(
                                     proxy_dbus,
                                     "org.freedesktop.DBus.GetConnectionUnixProcessID",
                                     g_variant_new("(s)", owner),
                                     G_DBUS_CALL_FLAGS_NONE,
                                     /* It's not worth to wait for the PID
                                      * longer than half a second when dying */
                                     500,
                                     NULL,
                                     &error);

        if (error) {
                g_error_free(error);
                return false;
        }

        g_variant_get(pidinfo, "(u)", &pid);

        g_object_unref(proxy_fdn);
        g_object_unref(proxy_dbus);
        g_free(owner);
        if (daemoninfo)
                g_variant_unref(daemoninfo);
        if (pidinfo)
                g_variant_unref(pidinfo);

        return true;
}


static void on_name_lost(GDBusConnection *connection,
                         const gchar *name,
                         gpointer user_data)
{
        if (connection) {
                char *name;
                int pid;
                if (dbus_get_fdn_daemon_info(connection, &pid, &name, NULL)) {
                        DIE("Cannot acquire '"FDN_NAME"': "
                            "Name is acquired by '%s' with PID '%d'.", name, pid);
                } else {
                        DIE("Cannot acquire '"FDN_NAME"'.");
                }
        } else {
                DIE("Cannot connect to DBus.");
        }
        exit(1);
}

static struct raw_image *get_raw_image_from_data_hint(GVariant *icon_data)
{
        struct raw_image *image = g_malloc(sizeof(struct raw_image));
        GVariant *data_variant;
        gsize expected_len;

        g_variant_get(icon_data,
                      "(iiibii@ay)",
                      &image->width,
                      &image->height,
                      &image->rowstride,
                      &image->has_alpha,
                      &image->bits_per_sample,
                      &image->n_channels,
                      &data_variant);

        expected_len = (image->height - 1) * image->rowstride + image->width
                * ((image->n_channels * image->bits_per_sample + 7) / 8);

        if (expected_len != g_variant_get_size (data_variant)) {
                LOG_W("Expected image data to be of length %" G_GSIZE_FORMAT
                      " but got a length of %" G_GSIZE_FORMAT,
                      expected_len,
                      g_variant_get_size(data_variant));
                g_free(image);
                g_variant_unref(data_variant);
                return NULL;
        }

        image->data = (guchar *) g_memdup(g_variant_get_data(data_variant),
                                          g_variant_get_size(data_variant));
        g_variant_unref(data_variant);

        return image;
}

int dbus_init(void)
{
        guint owner_id;

        introspection_data = g_dbus_node_info_new_for_xml(introspection_xml,
                                                          NULL);

        owner_id = g_bus_own_name(G_BUS_TYPE_SESSION,
                                  FDN_NAME,
                                  G_BUS_NAME_OWNER_FLAGS_NONE,
                                  on_bus_acquired,
                                  on_name_acquired,
                                  on_name_lost,
                                  NULL,
                                  NULL);

        return owner_id;
}

void dbus_teardown(int owner_id)
{
        g_clear_pointer(&introspection_data, g_dbus_node_info_unref);

        g_bus_unown_name(owner_id);
}

/* vim: set tabstop=8 shiftwidth=8 expandtab textwidth=0: */
