#ifndef ARM_PROJECT_GEOMETRYLOADER_H
#define ARM_PROJECT_GEOMETRYLOADER_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QString>

#include "object.h"
#include "filemanager.h"


class GeometryLoader : protected QOpenGLFunctions {

public:
    GeometryLoader(QString path);
    ~GeometryLoader();

    void drawGeometry(QOpenGLShaderProgram *program, float alpha);
    void saveGeometry(QString path);

private:
    void initGeometry(int maxValue);

    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    Object * obj;
};


#endif //ARM_PROJECT_GEOMETRYLOADER_H
