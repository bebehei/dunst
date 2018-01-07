#include "../src/utils.h"
#include "test.h"

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define SUITE_NAME "utils"

typedef struct struct_string_strip_delimited { const char *desc; char a; char b; const char *bug; const char *i; const char *exp; } data_string_strip_delimited;

void helper_string_strip_delimited(gconstpointer data)
{
        data_string_strip_delimited *d = (data_string_strip_delimited*) data;

        g_test_bug(d->bug);

        char *out = g_strdup(d->i);

        string_strip_delimited(out, d->a, d->b);

        g_assert_cmpstr(out, ==, d->exp);
        g_free(out);
}

#define THIS_SUITE_NAME "string_strip_delimited"
SUITE_BEGIN(string_strip_delimited)

        data_string_strip_delimited datasets[] = {
                        { .a = '<', .b = '>', .i = "A <simple> string_strip_delimited test",
                                              .exp = "A  string_strip_delimited test",                            .bug = "", .desc = PREFIX"/simple tag"    },
                        { .a = '<', .b = '>', .i = "Remove <blink>html <b><i>tags</i></b></blink>",
                                              .exp = "Remove html tags",                                          .bug = "", .desc = PREFIX"/multiple tags"},
                        { .a = '|', .b = '|', .i = "Calls|with|identical|delimiters|are|handled|properly",
                                              .exp = "Calls",                                                     .bug = "", .desc = PREFIX"/identical delimiters" },
                        { .a = '<', .b = '>', .i = "<Return empty string if there is nothing left>",
                                              .exp = "",                                                          .bug = "", .desc = PREFIX"/tag spans everything" },
                        { .a = '<', .b = '>', .i = "Nothing is done if there are no delimiters in the string",
                                              .exp = "Nothing is done if there are no delimiters in the string",  .bug = "", .desc = PREFIX"/no delimiters in text" },
        };

SUITE_END(string_strip_delimited)
#undef THIS_SUITE_NAME

typedef struct struct_string_to_path { const char *desc; const char *bug; const char *i; const char *exp; } data_string_to_path;

void helper_string_to_path(gconstpointer data)
{
        data_string_to_path *d = (data_string_to_path*) data;

        g_test_bug(d->bug);

        char *exp = string_replace_all("HOME", getenv("HOME"), g_strdup(d->exp));
        char *out = string_to_path(g_strdup(d->i));

        g_assert_cmpstr(out, ==, exp);
        g_free(exp);
        g_free(out);
}

#define THIS_SUITE_NAME "string_to_path"
SUITE_BEGIN(string_to_path)

        data_string_to_path datasets[] = {
                        { .i = "/usr/local/bin/script",  .exp = "/usr/local/bin/script",     .bug = "", .desc = PREFIX"/no tilde"    },
                        { .i = "~path/with/wrong/tilde", .exp = "~path/with/wrong/tilde",    .bug = "", .desc = PREFIX"/wrong tilde"},
                        { .i = "~/.path/with/tilde",     .exp = "HOME/.path/with/tilde",     .bug = "", .desc = PREFIX"/correct tilde" },
                        { .i = "~/path and some space",  .exp = "HOME/path and some space",  .bug = "", .desc = PREFIX"/correct tilde and some space" },
        };

SUITE_END(string_to_path)
#undef THIS_SUITE_NAME

typedef struct struct_string_to_time { ExpectedLogMessage msg; const char *desc; const char *bug; const char *i; gint64 exp; } data_string_to_time;

void helper_string_to_time(gconstpointer data)
{
        data_string_to_time *d = (data_string_to_time*) data;
        g_test_bug(d->bug);
        if (d->msg.message_regex) g_test_expect_message(d->msg.domain, d->msg.level, d->msg.message_regex);
        gint64 out = string_to_time(d->i);
        if (d->msg.message_regex) g_test_assert_expected_messages();
        g_assert_cmpint(out, ==, 1000 * d->exp);
}

#define THIS_SUITE_NAME "string_to_time"
SUITE_BEGIN(string_to_time)

        ExpectedLogMessage nomsg = {.message_regex = NULL};
        ExpectedLogMessage digit = {.message_regex = "*No digits found*", .domain = NULL, .level = G_LOG_LEVEL_WARNING};

        data_string_to_time datasets[] = {
                        { .i = "5000 ms",      .exp = 5000,      .msg = nomsg, .bug = "", .desc = PREFIX"/space between args"    },
                        { .i = "  5    ms   ", .exp = 5,         .msg = nomsg, .bug = "", .desc = PREFIX"/arbitary spaces around"},
                        { .i = "     ms   ",   .exp = 0,         .msg = digit, .bug = "", .desc = PREFIX"/no digits found"       },
                        { .i = "  d9",         .exp = 0,         .msg = digit, .bug = "", .desc = PREFIX"/flipped char and unit" },
                        { .i = "5000ms",       .exp = 5000,      .msg = nomsg, .bug = "", .desc = PREFIX"/no space between args" },
                        { .i = "10",           .exp = 10000,     .msg = nomsg, .bug = "", .desc = PREFIX"/default unit seconds"  },
                        { .i = "2m",           .exp = 120000,    .msg = nomsg, .bug = "", .desc = PREFIX"/timeunit minutes"      },
                        { .i = "11h",          .exp = 39600000,  .msg = nomsg, .bug = "", .desc = PREFIX"/timeunit hours"        },
                        { .i = "9d",           .exp = 777600000, .msg = nomsg, .bug = "", .desc = PREFIX"/timeunit days"         },
        };
SUITE_END(string_to_time)
#undef THIS_SUITE_NAME

// Main Suite
#define THIS_SUITE_NAME "utils"
SUITE_BEGIN(utils)
        SUITE_ADD(string_to_time);
        SUITE_ADD(string_to_path);
        SUITE_ADD(string_strip_delimited);
SUITE_RETURN
#undef THIS_SUITE_NAME

/* vim: set tabstop=8 shiftwidth=8 expandtab textwidth=0: */
