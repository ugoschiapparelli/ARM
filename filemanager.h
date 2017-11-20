#ifndef FILEMANAGER_H
#define FILEMANAGER_H


#include "object.h"

class FileManager
{
public:
    static Object * loadPGM3D(const char* path);
    static Object * loadOBJ(const char* path, int *maxValue);
    static void saveOBJ(const char* path, Object * obj);
};


#endif // FILEMANAGER_H
