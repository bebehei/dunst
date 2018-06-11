#include "greatest.h"
#include "src/utils.h"

#include <glib.h>

TEST test_string_replace_char(void)
{
        char *text = malloc(128 * sizeof(char));
        strcpy(text, "a aa aaa");
        ASSERT_STR_EQ("b bb bbb", string_replace_char('a', 'b', text));

        strcpy(text, "Nothing to replace");
        ASSERT_STR_EQ("Nothing to replace", string_replace_char('s', 'a', text));

        strcpy(text, "");
        ASSERT_STR_EQ("", string_replace_char('a', 'b', text));
        free(text);

        PASS();
}

/*
 * We trust that string_replace_all and string_replace properly reallocate
 * memory if the result is longer than the given string, no real way to test for
 * that far as I know.
 */

TEST test_string_replace_all(void)
{

        char *text = malloc(128 * sizeof(char));
        strcpy(text, "aaaaa");
        ASSERT_STR_EQ("bbbbb", (text = string_replace_all("a", "b", text)));

        strcpy(text, "");
        ASSERT_STR_EQ("", (text = string_replace_all("a", "b", text)));

        strcpy(text, "Nothing to replace");
        ASSERT_STR_EQ((text = string_replace_all("z", "a", text)), "Nothing to replace");

        strcpy(text, "Reverse this");
        ASSERT_STR_EQ("Reverse sith", (text = string_replace_all("this", "sith", text)));

        strcpy(text, "abcdabc");
        ASSERT_STR_EQ("xyzabcdxyzabc", (text = string_replace_all("a", "xyza", text)));

        free(text);
        PASS();
}

TEST test_string_replace(void)
{
        char *text = malloc(128 * sizeof(char));
        strcpy(text, "aaaaa");
        ASSERT_STR_EQ("baaaa", (text = string_replace("a", "b", text)) );

        strcpy(text, "");
        ASSERT_STR_EQ((text = string_replace("a", "b", text)), "");

        strcpy(text, "Nothing to replace");
        ASSERT_STR_EQ((text = string_replace("z", "a", text)), "Nothing to replace");

        strcpy(text, "Reverse this");
        ASSERT_STR_EQ("Reverse sith", (text = string_replace("this", "sith", text)));

        strcpy(text, "abcdabc");
        ASSERT_STR_EQ("xyzabcdabc", (text = string_replace("a", "xyza", text)));

        free(text);
        PASS();
}

TEST test_string_append(void)
{
        char *exp;

        ASSERT_STR_EQ("text_sep_bit", (exp = string_append(g_strdup("text"), "bit", "_sep_")));
        g_free(exp);
        ASSERT_STR_EQ("textbit", (exp = string_append(g_strdup("text"), "bit", NULL)));
        g_free(exp);
        ASSERT_STR_EQ("textbit", (exp = string_append(g_strdup("text"), "bit", "")));
        g_free(exp);

        ASSERT_STR_EQ("text", (exp = string_append(g_strdup("text"), "", NULL)));
        g_free(exp);
        ASSERT_STR_EQ("text", (exp = string_append(g_strdup("text"), "", "_sep_")));
        g_free(exp);

        ASSERT_STR_EQ("b", (exp = string_append(g_strdup(""), "b", NULL)));
        g_free(exp);
        ASSERT_STR_EQ("b", (exp = string_append(NULL, "b", "_sep_")));
        g_free(exp);

        ASSERT_STR_EQ("a", (exp = string_append(g_strdup("a"), "", NULL)));
        g_free(exp);
        ASSERT_STR_EQ("a", (exp = string_append(g_strdup("a"), NULL, "_sep_")));
        g_free(exp);

        ASSERT_STR_EQ("", (exp = string_append(g_strdup(""), "", "_sep_")));
        g_free(exp);
        ASSERT_EQ(NULL, (exp = string_append(NULL, NULL, "_sep_")));
        g_free(exp);

        PASS();
}

TEST test_string_strip_delimited(void)
{
        char *text = malloc(128 * sizeof(char));

        strcpy(text, "A <simple> string_strip_delimited test");
        string_strip_delimited(text, '<', '>');
        ASSERT_STR_EQ("A  string_strip_delimited test", text);

        strcpy(text, "Remove <blink>html <b><i>tags</i></b></blink>");
        string_strip_delimited(text, '<', '>');
        ASSERT_STR_EQ("Remove html tags", text);

        strcpy(text, "Calls|with|identical|delimiters|are|handled|properly");
        string_strip_delimited(text, '|', '|');
        ASSERT_STR_EQ("Calls", text);

        strcpy(text, "<Return empty string if there is nothing left>");
        string_strip_delimited(text, '<', '>');
        ASSERT_STR_EQ("", text);

        strcpy(text, "Nothing is done if there are no delimiters in the string");
        string_strip_delimited(text, '<', '>');
        ASSERT_STR_EQ("Nothing is done if there are no delimiters in the string", text);

        free(text);
        PASS();
}

TEST test_string_to_path(void)
{
        char *ptr, *exp;
        char *home = getenv("HOME");

        exp = "/usr/local/bin/script";
        ASSERT_STR_EQ(exp, (ptr = string_to_path(g_strdup(exp))));
        free(ptr);

        exp = "~path/with/wrong/tilde";
        ASSERT_STR_EQ(exp, (ptr = string_to_path(g_strdup(exp))));
        free(ptr);

        ASSERT_STR_EQ((exp = g_strconcat(home, "/.path/with/tilde", NULL)),
                      (ptr = string_to_path(g_strdup("~/.path/with/tilde"))));
        free(exp);
        free(ptr);

        ASSERT_STR_EQ((exp = g_strconcat(home, "/.path/with/tilde and some space", NULL)),
                      (ptr = string_to_path(g_strdup("~/.path/with/tilde and some space"))));
        free(exp);
        free(ptr);

        PASS();
}

TEST test_string_to_time(void)
{
        char *input[] = { "5000 ms", "5000ms",  "100", "10s",   "2m",    "11h",      "9d", "   5 ms   ", NULL };
        gint64  exp[] = {      5000,     5000, 100000, 10000, 120000, 39600000, 777600000,     5,         0};

        int i = 0;
        while (input[i]){
                ASSERT_EQ_FMT(string_to_time(input[i], -1), exp[i]*1000, "%ld");
                i++;
        }

        PASS();
}

TEST test_string_parse_bool(void)
{
        ASSERT(string_parse_bool("true", false));
        ASSERT(string_parse_bool("True", false));
        ASSERT(string_parse_bool("TRUE", false));

        ASSERT(string_parse_bool("y", false));
        ASSERT(string_parse_bool("yes", false));
        ASSERT(string_parse_bool("Yes", false));
        ASSERT(string_parse_bool("YES", false));

        ASSERT_FALSE(string_parse_bool("false", true));
        ASSERT_FALSE(string_parse_bool("False", true));
        ASSERT_FALSE(string_parse_bool("FALSE", true));

        ASSERT_FALSE(string_parse_bool("n", true));
        ASSERT_FALSE(string_parse_bool("no", true));
        ASSERT_FALSE(string_parse_bool("No", true));
        ASSERT_FALSE(string_parse_bool("NO", true));

        /* Assert that default values get taken
         * properly on invalid input
         */
        ASSERT(string_parse_bool(NULL, true) == true);
        ASSERT(string_parse_bool(NULL, false) == false);

        ASSERT(string_parse_bool("", true) == true);
        ASSERT(string_parse_bool("", false) == false);

        ASSERT(string_parse_bool("Boolean", false) == false);
        ASSERT(string_parse_bool("Boolean", false) == false);

        PASS();
}

SUITE(suite_utils)
{
        RUN_TEST(test_string_replace_char);
        RUN_TEST(test_string_replace_all);
        RUN_TEST(test_string_replace);
        RUN_TEST(test_string_append);
        RUN_TEST(test_string_strip_delimited);
        RUN_TEST(test_string_to_path);
        RUN_TEST(test_string_to_time);
        RUN_TEST(test_string_parse_bool);
}
/* vim: set tabstop=8 shiftwidth=8 expandtab textwidth=0: */
