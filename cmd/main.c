/* copyright 2012 - 2013 Sascha Kruse and contributors (see LICENSE for licensing information) */

#include "main.h"

#include <glib.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cmd_help.h"
#include "cmd_status.h"
#include "utils.h"

static gboolean settings_version = false;

static GOptionEntry options[] = {
    { "version", '\0', 0, G_OPTION_ARG_NONE, &settings_version, "Print version of dunstcmd", NULL },
    { NULL }
};

enum subcmd parse_subcommand(const char *cmd)
{
        if (STR_EQ(cmd, ""))
                return DUNSTCMD_MAIN;
        if (STR_EQ(cmd, "help"))
                return DUNSTCMD_SUBCMD_HELP;
        if (STR_EQ(cmd, "status"))
                return DUNSTCMD_SUBCMD_STATUS;

        return DUNSTCMD_SUBCMD_INVALID;
}

const char *cmdlist =
        "Available subcommands:\n"
        "\n"
        " - help:   Print help for other subcommands\n"
        " - status: Get status information about running dunst instance\n"
        "";

int main(int argc, char *argv[])
{
        GError *err = NULL;

        GOptionContext *context = g_option_context_new("<SUBCOMMAND>");
        g_option_context_add_main_entries(context, options, NULL);
        g_option_context_set_summary(context, "Send commands to the dunst notification daemon");
        g_option_context_set_description(context, cmdlist);
        /* Stop at the first non-option (first command)
         * and pass argv further to the subcommand */
        g_option_context_set_strict_posix(context, true);

        if (!g_option_context_parse(context, &argc, &argv, &err)) {
                DIE("Parsing main options failed: %s", err->message);
        }

        if (settings_version) {
                g_print("dunstcmd " VERSION "\n");
                exit(EXIT_SUCCESS);
        }

        if (argc < 2) {
                DIE("No command given. Use '--help' to print help output.");
        }

        char *command = argv[1];
        char **v = argv+1;
        int c = argc-1;
        switch (parse_subcommand(command)) {
        case DUNSTCMD_SUBCMD_HELP:
                main_subcmd_help(c, v);
                break;
        case DUNSTCMD_SUBCMD_STATUS:
                main_subcmd_status(c, v);
                break;
        case DUNSTCMD_MAIN:
        case DUNSTCMD_SUBCMD_INVALID:
                DIE("Invalid subcommand: '%s'", command);
        }

        exit(EXIT_SUCCESS);
}

/* vim: set tabstop=8 shiftwidth=8 expandtab textwidth=0: */
