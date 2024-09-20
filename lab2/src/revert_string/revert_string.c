#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "revert_string.h"

void RevertString(char *str)
{
	int first = 0;
    int last = strlen(str) - 1;
    char temp;

    // Swap characters till first and last meet
    while (first < last) {
      
        // Swap characters
        temp = str[first];
        str[first] = str[last];
        str[last] = temp;

        // Move pointers towards each other
        first++;
        last--;
    }
}

