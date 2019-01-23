#include <stdbool.h>
#include <glib.h>
#include <gio/gio.h>

struct signal_actioninvoked {
        guint id;
        gchar *key;
        guint subscription_id;
        GDBusConnection *conn;
};

struct signal_closed {
        guint32 id;
        guint32 reason;
        guint subscription_id;
        GDBusConnection *conn;
};

struct dbus_notification {
        const char* app_name;
        guint replaces_id;
        const char* app_icon;
        const char* summary;
        const char* body;
        GHashTable *actions;
        GHashTable *hints;
        int expire_timeout;
};

/* Methods to (un)subscribe to specified DBus signals */
void dbus_signal_subscribe_actioninvoked(struct signal_actioninvoked *actioninvoked);
void dbus_signal_unsubscribe_actioninvoked(struct signal_actioninvoked *actioninvoked);
void dbus_signal_subscribe_closed(struct signal_closed *closed);
void dbus_signal_unsubscribe_closed(struct signal_closed *closed);

/* Low level method to call an arbitary FDN Dbus method with the params given in the GVariant */
GVariant *dbus_invoke(const char *method, GVariant *params);

/* Assemble and send a notification over the DBus wire */
struct dbus_notification *dbus_notification_new(void);
void dbus_notification_free(struct dbus_notification *n);
bool dbus_notification_fire(struct dbus_notification *n, uint *id);
void dbus_notification_set_raw_image(struct dbus_notification *n_dbus, const char *path);

/* vim: set tabstop=8 shiftwidth=8 expandtab textwidth=0: */
