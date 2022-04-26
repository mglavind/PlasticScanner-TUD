#include "assert.h" 
#include "Arduino.h"

void assertion_failure(const char* expr, const char* file, int linenum)
{
    Serial.print("Assertion failed in '");
    Serial.print(file);
    Serial.print("' on line ");
    Serial.print(linenum);
    Serial.print(": ");
    Serial.println(expr);
    Serial.flush();

    while (1);
}