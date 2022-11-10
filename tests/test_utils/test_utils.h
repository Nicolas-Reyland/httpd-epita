#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#define EXP_GOT_DIGIT "Expected %d. Got %d."
#define EXP_GOT_DIGIT_ZU "Expected %zu. Got %zu."
#define EXP_GOT_STR "Expected '%s'. Got '%s'."
#define EXP_GOT_STR_MULTI_LINE "Expected :\n''''%s'''\n\nGot :\n'''%s'''\n"
#define EXP_NOT_NULL "Expected NOt null."

#define CR_ASSERT_NULL_EXPANDED(Val)                                           \
    cr_assert_null((Val),                                                      \
                   #Val ": "                                                   \
                        "Expected " #Val " TO BE null.")

#define CR_ASSERT_NOT_NULL_EXPANDED(Val)                                       \
    cr_assert_not_null((Val),                                                  \
                       #Val ": "                                               \
                            "Expected " #Val " NOT null.")

#define CR_ASSERT_EQ_DIGIT_EXPANDED(Val1, Val2)                                \
    cr_assert_eq((Val1), (Val2), #Val2 ": " EXP_GOT_DIGIT, (Val1), (Val2))

#define CR_ASSERT_EQ_DIGIT_ZU_EXPANDED(Val1, Val2)                             \
    cr_assert_eq((Val1), (Val2), #Val2 ": " EXP_GOT_DIGIT_ZU, (Val1), (Val2))

#define CR_ASSERT_STR_EQ_EXPANDED(Val1, Val2)                                  \
    cr_assert_str_eq((Val1), (Val2), #Val2 ": " EXP_GOT_STR, (Val1), (Val2))

#endif /* TEST_UTILS_H */
