/*
    Class: ECET 4640-002
    Assignment: Lab Assignment 3
    Authors: Christian Messmer, Karl Miller, Paul Shriner

    colors.h: Define color macros for use with printing text to the console

    Acknowledgements/Credits:
        1. https://www.man7.org/linux/man-pages/man4/console_codes.4.html
*/

#ifndef colors_h
#define colors_h
/*
    Karl's magic color macros.

    These use Virtual Terminal escape sequences to trigger color changes on the console when printed.

    See 1 in Acknowledgements/Credits for more information.

*/
#define COLOR_RED "\e[38;2;255;75;75m"
#define COLOR_BLUE "\e[38;2;0;240;240m"
#define COLOR_GREEN "\e[38;2;0;240;0m"
#define COLOR_YELLOW "\e[38;2;255;255;0m"
#define COLOR_GRAY "\e[38;2;224;224;224m"
#define COLOR_BOLD "\e[1m"
#define COLOR_RESET "\e[0m"

#endif
