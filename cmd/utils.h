/* copyright 2013 Sascha Kruse and contributors (see LICENSE for licensing information) */

#ifndef CMD_UTILS_H
#define CMD_UTILS_H

#include <stdlib.h>
#include <string.h>

/* Taken from src/log.h */
#define LOG_E(...) do { g_printerr(__VA_ARGS__); g_printerr("\n"); exit(EXIT_FAILURE); } while(0)
#define LOG_C(...) do { g_printerr(__VA_ARGS__); g_printerr("\n"); } while(0)
#define LOG_W(...) do { g_printerr(__VA_ARGS__); g_printerr("\n"); } while(0)
#define LOG_M(...) do { g_printerr(__VA_ARGS__); g_printerr("\n"); } while(0)
#define LOG_I(...) do { g_printerr(__VA_ARGS__); g_printerr("\n"); } while(0)
#define LOG_D(...) do { g_printerr(__VA_ARGS__); g_printerr("\n"); } while(0)

#define DIE(...) do { LOG_C(__VA_ARGS__); exit(EXIT_FAILURE); } while (0)

//TODO: remove (or take from dunstmain?)
//! Test if string a and b contain the same chars
#define STR_EQ(a, b) (strcmp(a, b) == 0)

#endif
/* vim: set tabstop=8 shiftwidth=8 expandtab textwidth=0: */
