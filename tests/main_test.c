#include <stdio.h>

#include "map_test.h"
#include "Util_test.h"
#include "crypt_test.h"

void RunAllTests(void)
{
    CuString *output = CuStringNew();
    CuSuite *suite = CuSuiteNew();
    
    CuSuiteAddSuite(suite, mapTestGetSuite());
    CuSuiteAddSuite(suite, utilTestGetSuite());
    CuSuiteAddSuite(suite, cryptTestGetSuite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}

int main() {
    RunAllTests();
    return 0;
}
