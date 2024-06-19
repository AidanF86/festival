/* date = March 23rd 2024 2:30 pm */

#ifndef CCC_BASE_H
#define CCC_BASE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define True 1
#define False 0

typedef float f32;
typedef double f64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int32_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef u32 b32;
typedef u64 b64;

typedef Color color;

// From https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
#define AnsiColor_Red     "\x1b[31m"
#define AnsiColor_Green   "\x1b[32m"
#define AnsiColor_Yellow  "\x1b[33m"
#define AnsiColor_Blue    "\x1b[34m"
#define AnsiColor_Magenta "\x1b[35m"
#define AnsiColor_Cyan    "\x1b[36m"
#define AnsiColor_Reset   "\x1b[0m"

#define _Assert(X) if(!X) {\
char *NullAddr = (char *) 0; *NullAddr = 0;\
}

#define printerror(...) printf("%s", AnsiColor_Red); printf("ERROR (%s, %s, line %d): ", __FILE__, __func__, __LINE__); printf(__VA_ARGS__); printf("%s\n", AnsiColor_Reset);
#define printwarning(...) printf("%s", AnsiColor_Yellow); printf("WARNING (%s, %s, line %d): ", __FILE__, __func__, __LINE__); printf(__VA_ARGS__); printf("%s\n", AnsiColor_Reset); _Assert(false);

#define Assert(X) if(!X) {\
printerror("Assert Called by: %s, %s, line %d", __FILE__, __func__, __LINE__);\
_Assert(X)\
}



void *
_TryMalloc(u64 Size, const char *CalledByFunction, int CalledByLine, const char *CalledByFile)
{
    void *Result = (void *)malloc(Size);
    if(Result != NULL)
    {
        return Result;
    }
    else
    {
        printerror("failed malloc!");
        printerror("\tCalled by: %s, %s, line %d", CalledByFile, CalledByFunction, CalledByLine);
        return NULL;
    }
}

void *
_TryRealloc(void *Data, u64 Size, const char *CalledByFunction, int CalledByLine, const char *CalledByFile)
{
    void *Result = (void *)realloc(Data, Size);
    if(Result != NULL)
    {
        return Result;
    }
    else
    {
        printerror("ERROR (_TryMalloc): failed malloc!");
        printerror("\tCalled by: %s, %s, line %d", CalledByFile, CalledByFunction, CalledByLine);
        return NULL;
    }
}

void
_TryFree(void *Data, const char *CalledByFunction, int CalledByLine, const char *CalledByFile)
{
    //printerror("Freeing data: called by : %s, %s, line %d", CalledByFile, CalledByFunction, CalledByLine);
    free(Data);
}

#define TryMalloc(Size) _TryMalloc(Size, __func__, __LINE__, __FILE__)
#define TryRealloc(Data, Size) _TryRealloc(Data, Size, __func__, __LINE__, __FILE__)
#define TryFree(Data) _TryFree(Data, __func__, __LINE__, __FILE__)


struct v2
{
    union
    {
        struct
        {
            int x, y;
        };
        struct
        {
            int w, h;
        };
    };
};
struct rect
{
    int x, y, w, h;
};

Rectangle
R(rect a)
{
    Rectangle Result = {(f32)a.x, (f32)a.y, (f32)a.w, (f32)a.h};
    return Result;
}



inline Vector2
V(v2 P)
{
    Vector2 Result = {(f32)P.x, (f32)P.y};
    return Result;
}

inline rect
Rect(int x, int y, int w, int h)
{
    rect Result;
    Result.x = x;
    Result.y = y;
    Result.w = w;
    Result.h = h;
    return Result;
}


struct buffer_pos {
    // line, col
    int l, c;
};


inline buffer_pos
BufferPos(int l, int c) {
    buffer_pos Result;
    Result.l = l;
    Result.c = c;
    return Result;
}





#endif //CCC_BASE_H