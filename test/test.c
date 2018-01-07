#include <glib.h>

#include "test.h"
#include <stdbool.h>
#include "src/log.h"

SUITE_EXTERN(utils);

int main(int argc, char *argv[]) {
        dunst_log_init(false);
        g_test_init(&argc, &argv, NULL);

        //TODO: does this work?!
        g_test_bug_base ("https://github.com/dunst-project/dunst/issues/");

        GTestSuite *cur_suite = g_test_get_root();

        SUITE_ADD(utils);

        g_test_set_nonfatal_assertions();

        return g_test_run();
}
/* vim: set tabstop=8 shiftwidth=8 expandtab textwidth=0: */
