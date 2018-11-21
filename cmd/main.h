/* copyright 2013 Sascha Kruse and contributors (see LICENSE for licensing information) */

#ifndef CMD_MAIN_H
#define CMD_MAIN_H

enum subcmd {
        DUNSTCMD_MAIN,
        DUNSTCMD_SUBCMD_HELP,
        DUNSTCMD_SUBCMD_INVALID,
};

int main(int argc, char *argv[]);

enum subcmd parse_subcommand(const char *cmd);

#endif
/* vim: set tabstop=8 shiftwidth=8 expandtab textwidth=0: */
