/*
    Class: ECET 4640-002
    Assignment: Lab Assignment 3
    Authors: Christian Messmer, Karl Miller, Paul Shriner

    Cipher.c: Functions used for generating the cipher, printing it out, and encrypting a string.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "colors.h"

/**
    @private
    @param array The array to fill.
    @param start The character to start at.
    @param end The character to end at.
    Fills a character array with sequential values
    Used by GenerateCipher
    @attention precondition: array is at least end-start length
    @attention precondition: start < end
*/
void FillArraySequential(char *array, char start, char end)
{
    char length = end - start + 1;
    char i;
    for (i = start; i <= end; i++)
    {
        array[i - start] = i;
    }
}

// See Cipher.h for header comments
void GenerateCipher(char *cipher, char start, char end)
{
    time_t t;
    srand((unsigned)time(&t));

    FillArraySequential(cipher, start, end);
    char length = end - start + 1;
    char hold, swap_index, i;
    for (i = 0; i < length; i++)
    {
        swap_index = rand() % length;
        hold = cipher[swap_index];
        cipher[swap_index] = cipher[i];
        cipher[i] = hold;
    }
}

// See Cipher.h for header comments
void PrintCipher(char *cipher, char start, char length)
{
    printf("\nCipher:\n");
    char i;
    for (i = 0; i < length; i++)
    {
        printf("%s\'%s%c%s\'%s \u00BB %s\'%s%c%s\'%s      ", COLOR_GRAY, COLOR_RED, i + start, COLOR_GRAY, COLOR_RESET, COLOR_GRAY, COLOR_BLUE, cipher[i], COLOR_GRAY, COLOR_RESET);
        if ((i + 1) % 5 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
}

// See Cipher.h for header comments
void EncryptString(char *string, int length, char *cipher, char start, char end)
{

    char cipher_l = end - start + 1;
    int i;
    for (i = 0; i < length; i++)
    {
        if (!(string[i] - start > end || string[i] < start))
        {
            string[i] = cipher[string[i] - start];
        }
    }
}

void DecryptString(char* string, int length, char* cipher, char start, char end) {
    char cipher_l = end - start + 1;
    
    int i;
    for(i = 0; i < length; i++) {
        if(!(string[i] - start > end || string[i] < start)) {
            char* c = strchr(cipher, string[i]);
            int index = c - string;

            string[i] = cipher[index];
        }
    }
}
