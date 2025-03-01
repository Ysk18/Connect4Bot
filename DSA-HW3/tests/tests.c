#include <munit.h>
#include <stdio.h>
#include "tree_tests.h"
#include "game_bot_tests.h"
#include "interface_tests.h"

// Define the main test suite that includes all component suites
static const MunitSuite test_suites[] = {
    {"/tree", tree_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/game_bot", game_bot_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/interface", interface_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE}};

static const MunitSuite test_suite = {
    "tests",
    NULL,
    test_suites,
    1,
    MUNIT_SUITE_OPTION_NONE};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
    return munit_suite_main(&test_suite, NULL, argc, argv);
}