#include "crypt_test.h"
#include <string.h>
#include "Cipher.h"

void BasicTests(CuTest *tc) 
{
    // cipher from file
    char * cipher = "`\"G)YF7A,R2L'@ ZD/E5I<?H:i4NJ&g;rB(f#KobljnW1C{_-Ua]%^cV\\>tOP|pQ$689=+whzS3*Xm!ek~My[}sqduv0.Tx";
    //size_t len_cipher = strlen(cipher);
    // start character from file
    char start = ' ';
    // end character from file
    char end = '~';

    char* to_encrypt = " ~";
    char* decrypted = " ~";
    char* expected = "`x";

    EncryptString(to_encrypt, 2, cipher, start, end);
    // CuAssertIntEquals(tc, expected[0], to_encrypt[0]);
    // CuAssertIntEquals(tc, expected[1], to_encrypt[1]);
    // DecryptString(to_encrypt, 2, cipher, start, end);
    // CuAssertIntEquals(tc, decrypted[0], to_encrypt[0]);
    // CuAssertIntEquals(tc, decrypted[1], to_encrypt[1]);

}


CuSuite * cryptTestGetSuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, BasicTests);
    return suite;
}
