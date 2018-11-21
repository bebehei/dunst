/* copyright 2012 - 2013 Sascha Kruse and contributors (see LICENSE for licensing information) */

#include "cmd_status.h"

#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stdio.h>

#include "main.h"
#include "utils.h"

static bool settings_listen = false;
static char *settings_set = NULL;

static GOptionEntry options[] = {
    { "listen", 'l', 0, G_OPTION_ARG_NONE, &settings_listen, "Listen for status changes", NULL },
    { "set", 's', 0, G_OPTION_ARG_STRING, &settings_set, "Listen for status changes", NULL },
    { NULL }
};

static void status_print(bool status)
{
        if (status)
                printf("running\n");
        else
                printf("paused\n");
}

static bool cmd_status_retrieve(GError **error)
{
        return false;
}

static bool cmd_status_set(bool value)
{
        return false;
}

void main_subcmd_status(int argc, char *argv[])
{
        GError *err = NULL;

        GOptionContext *context = g_option_context_new("status [STATUS OPTIONS]");
        g_option_context_add_main_entries(context, options, NULL);
        g_option_context_set_summary(context, "Query the status of the running dunst instance.");

        if (!g_option_context_parse(context, &argc, &argv, &err)) {
                DIE("Option parsing failed: %s\n", err->message);
        }

        if (settings_set) {
                bool status;
                if (parse_bool(settings_set, &status))
                        cmd_status_set(status);
                else
                        DIE("Invalid boolean value: %s", settings_set);
        } else {
                bool status = cmd_status_retrieve(&err);
                if (err)
                        DIE("Cannot retrieve status: %s", err->message);
                status_print(status);
        }
}

/* vim: set tabstop=8 shiftwidth=8 expandtab textwidth=0: */
