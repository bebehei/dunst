
#define PREFIX "/"SUITE_NAME"/"THIS_SUITE_NAME

#define SUITE_BEGIN(name) \
GTestSuite *suite_##name(void) \
{ \
        GTestSuite *cur_suite = g_test_create_suite(#name); \

// I guess, the strings don't have to get copied!?
#define SUITE_END(name) \
        size_t len = sizeof(datasets)/sizeof(data_##name); \
        for (int i = 0; i < len; i++) { \
                data_##name *data = g_malloc(sizeof(data_##name)); \
                memcpy(data, &datasets[i], sizeof(data_##name)); \
                g_test_add_data_func(datasets[i].desc, data, helper_##name); \
        } \
        SUITE_RETURN

#define SUITE_RETURN \
        return cur_suite; \
}

#define SUITE_ADD(name) \
        g_test_suite_add_suite(cur_suite, suite_##name())
#define SUITE_EXTERN(name) \
        GTestSuite *suite_##name(void)

/* vim: set tabstop=8 shiftwidth=8 expandtab textwidth=0: */
