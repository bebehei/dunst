#include "../src/rules.c"
#include "greatest.h"

#include "../src/notification.h"

TEST test_rules_applyall(void)
{
        struct notification *n;
        struct rule *r;

        const char *tag = "testname";
        r = rule_new();
        r->set_stack_tag = g_strdup(tag);

        rules_add_rule(r);

        n = notification_create();
        notification_init(n);

        ASSERT_STR_EQ(n->stack_tag, r->stack_tag);
        ASSERT_STR_EQ(tag, r->stack_tag);
        ASSERT_STR_EQ(tag, n->stack_tag);
        
        notification_unref(n);
        rules_teardown();
        PASS();
}

TEST test_rules_notfree(void)
{
        rules_add_rule(&default_rules[0]);
        rules_teardown();
        PASS();
}

SUITE(suite_rules)
{
        RUN_TEST(test_rules_applyall);
        RUN_TEST(test_rules_notfree);
}

/* vim: set tabstop=8 shiftwidth=8 expandtab textwidth=0: */
