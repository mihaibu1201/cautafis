#ifndef __VERSIUNE_H_
#define __VERSIUNE_H_

#define APP_NUME    "cautafis"
#define VERSIUNE_MAJOR  1
#define VERSIUNE_MINOR  0
#define VERSIUNE_PATCH  1

#define STR_HELPER(x)   #x
#define STR(x)  STR_HELPER(x)

#define APP_VERSIUNE \
    STR(VERSIUNE_MAJOR) "."\
    STR(VERSIUNE_MINOR) "."\
    STR(VERSIUNE_PATCH)


#endif //__VERSIUNE_H_
