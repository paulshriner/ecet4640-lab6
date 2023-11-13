/**
 * @addtogroup Build
 * @{
*/
#include <stdlib.h>
#include <string.h>
#include "Build.h"
#include "Util.h"

User * CreateUsersArray(char ** userIDs, char ** userNames, int recordsCount)
{
    size_t uarr_size = sizeof(User) * recordsCount;
    User * uarr = malloc(uarr_size);
    memset(uarr, 0, uarr_size);
    int i;
    for(i = 0; i < recordsCount; i++)
    {
        strcpy(uarr[i].id, userIDs[i]);
        strcpy(uarr[i].name, userNames[i]);
    }
    return uarr;
}

map * CreateUsersMap(User * usersArray, int recordsCount)
{
    map * umap = NewMap(recordsCount * 3);
    int i;
    for(i = 0; i < recordsCount; i++) {
        Map_Set(umap, usersArray[i].id, &usersArray[i]);
        map_result mr = Map_Get(umap, usersArray[i].id);
        if(!mr.found) {
            printRed("Map failed on user: %s... your program may have issues.\n", usersArray[i].id);
        }
    }
    return umap;
}


/**
 * @}
*/