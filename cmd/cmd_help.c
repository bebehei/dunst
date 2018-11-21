/* copyright 2012 - 2013 Sascha Kruse and contributors (see LICENSE for licensing information) */

#include "cmd_help.h"

#include <glib.h>
#include <stdbool.h>
#include <stdio.h>

#include "main.h"
#include "utils.h"

static GOptionEntry options[] = {
    { NULL }
};

void main_subcmd_help(int argc, char *argv[])
{
        GError *err = NULL;

        GOptionContext *context = g_option_context_new("help [HELP OPTIONS]");
        g_option_context_add_main_entries(context, options, NULL);
        g_option_context_set_summary(context, "Display help for dunstcmd subcommands.");

        if (!g_option_context_parse(context, &argc, &argv, &err)) {
                DIE("Option parsing failed: %s\n", err->message);
        }

        char *command = argc < 2 ? "": argv[1];
        char *v[] = { argv[0], argc < 2 ? "--help": command, "--help" };
        int c = G_N_ELEMENTS(v);

        switch (parse_subcommand(command)) {
        case DUNSTCMD_MAIN:
                main(c, v);
                break;
        case DUNSTCMD_SUBCMD_HELP:
                main_subcmd_help(c, v);
                break;
        case DUNSTCMD_SUBCMD_INVALID:
                DIE("Cannot print help for '%s': Subcommand not available.", command);
        }
}

/* vim: set tabstop=8 shiftwidth=8 expandtab textwidth=0: */
