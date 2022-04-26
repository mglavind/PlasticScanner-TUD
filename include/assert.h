/*
    Modified/simplified version of the assert from:
    https://nicisdigital.wordpress.com/2011/07/05/assertions-in-microcontrollers/ 
*/
#ifndef _ASSERT_H_
#define _ASSERT_H_

void assertion_failure(const char* expr, const char* file, int linenum);

#define assert(expr) \
    if (expr) ; \
    else assertion_failure(#expr, __FILE__, __LINE__)


#endif /* _ASSERT_H_ */
