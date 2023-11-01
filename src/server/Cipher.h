#ifndef Cipher_h
#define Cipher_h
/*
    Class: ECET 4640-002
    Assignment: Lab Assignment 3
    Authors: Christian Messmer, Karl Miller, Paul Shriner

    Cipher.h: Function prototypes for Cipher.c
*/

/**
    @brief Generates a random cipher.

    Modular, intended to be reusable.

    @details The array is first filled with the characters between start and end. Then the array is traversed. Each element is swapped with some random other element. Each element is swapped at least once.

    @param cipher The cipher to fill.
    @param start The character to start.
    @param end The character the cipher will end on (inclusive).

    @attention Cipher is at least end-start in length.
    @attention Mutates: Fills cipher randomly with characters between start and end
*/
void GenerateCipher(char *cipher, char start, char end);

/**
    @brief Prints the cipher.

    Modular, intended to be reusable.

    @details Prints the cipher in a series of columns describing what each character will be transformed into.

    Uses colors.h.

    @param cipher The cipher to print.
    @param start The character started on cipher
    @param length The length of the cipher
*/
void PrintCipher(char *cipher, char start, char length);

/**
    @brief Encrypts the given string

    Modular, intended to be reusable.

    @details Replaces the string in place, mutating it. Anything out of bounds of the cipher will not be encrypted and will stay as its original character.

    @attention mutatates: encypts the string in place, destroying the original characters
    @param string String to ecrypt.
    @param length Length of `string`
    @param cipher Cipher to use for encrypting the string. Must be (start-end)+1 in size.
    @param start The first character the cipher uses
    @param end The last character the ciper uses
*/
void EncryptString(char *string, int length, char *cipher, char start, char end);

/**
    @brief Decrypts the given string

    Modular, intended to be reusable.

    @details Replaces the string in place, mutating it. Anything out of bounds of the cipher will not be decrypted and will stay as its original character.

    @attention mutatates: encypts the string in place, destroying the original characters
    @param string String to decrypt.
    @param length Length of `string`
    @param cipher Cipher to use for encrypting the string. Must be (start-end)+1 in size.
    @param start The first character the cipher uses
    @param end The last character the ciper uses
 */
void DecryptString(char* string, int length, char* cipher, char start, char end);
#endif
