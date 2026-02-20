#pragma once

#include<stdio.h>   // fread &Co.
#include<stdlib.h>  // malloc &Co.
#include<errno.h>   // errno
#include<string.h>  // strerror
#include<stdbool.h> // it returns bool, so...

// returns true on succesful read, false on failure during reading
bool read_entire_file(const char *path, char** into, size_t* file_size);

#ifdef READFILE_INCLUDE_IMPLEMENTATION

// stackoverflow was getting too much into the weeds of platform specific shit
// so I just stole (and readapted) this from nob.h
// https://github.com/tsoding/nob.h/blob/2210dccb978603e0deb4acd08ae3448b00f160f5/nob.h#L2429C1-L2464C2

// probably cross platform, I'm not installing wine to test it
bool read_entire_file(const char *path, char** into, size_t* file_size) {
    bool result = true;

    FILE *f = fopen(path, "rb");
    long long m = 0;
    if (f == NULL || fseek(f, 0, SEEK_END) < 0) {
        result = false;
        goto defer;
    }

#ifndef _WIN32
    m = ftell(f);
#else
    m = _telli64(_fileno(f));
#endif

    if (m < 0 || fseek(f, 0, SEEK_SET)) {
        result = false;
        goto defer;
    }

    // read file into null terminated string
    *into = (char*)malloc(m+1 * sizeof(char));
    fread((void*)*into, m, 1, f);
    (*into)[m] = '\0';

    if (ferror(f)) {
        // ferror does not set errno.
        // So the error reporting in defer is not correct in this case.
        result = false;
        goto defer;
    }

 defer:
    if (!result)
        fprintf(stderr, "Could not read file %s: %s", path, strerror(errno));
    else
        // returned size will be equivalent to strlen, so without the '\0'
        *file_size = m;
    if (f)
        fclose(f);
    return result;
}
#endif
