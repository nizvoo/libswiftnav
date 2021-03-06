#include <stdlib.h>
#include <check.h>

#include "check_suites.h"

int main(void)
{
  int number_failed;

  Suite *s = edc_suite();

  SRunner *sr = srunner_create(s);
  srunner_set_xml(sr, "test_results.xml");

  srunner_add_suite(sr, rtcm3_suite());
  srunner_add_suite(sr, bits_suite());
  srunner_add_suite(sr, sbp_suite());
  srunner_add_suite(sr, coord_system_suite());
  srunner_add_suite(sr, linear_algebra_suite());

  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

