 #include "geometryloader.h"
#include <QVector2D>
#include <QVector3D>
#include <QDebug>
#include <QFileInfo>

struct VertexData
{
    QVector3D position;
    float color;
};

GeometryLoader::GeometryLoader(QString path)
        : indexBuf(QOpenGLBuffer::IndexBuffer)
{
    initializeOpenGLFunctions();

    arrayBuf.create();

    QFileInfo fi(path);
    int maxValue = 0;
    if (fi.completeSuffix() == "obj")
    {
        obj = FileManager::loadOBJ(path.toStdString().c_str(), &maxValue);

    }
    else if (fi.completeSuffix() == "pgm3d")
    {
        obj = FileManager::loadPGM3D(path.toStdString().c_str());
        maxValue = 4;
    }
    initGeometry(maxValue);
}

GeometryLoader::~GeometryLoader()
{
    arrayBuf.destroy();
}

void
GeometryLoader::saveGeometry(QString path)
{
    FileManager::saveOBJ(path.toStdString().c_str(), obj);
}

void GeometryLoader::initGeometry(int maxValue)
{
    Face ** faces = obj->getFaces();
    unsigned int nbFaces = obj->getNbFaces();

    VertexData * vertices = (VertexData*)malloc(nbFaces*maxValue *sizeof(VertexData));

    for (unsigned int i = 0; i < nbFaces; ++i)
    {
        Face * f = faces[i];
        int nbVertex = f->getNbVertex();
        QVector3D prec;
        for (int j = 0; j < maxValue; ++j)
        {
            if (f->getColor() != 0)
            {
                VertexData vd;
                QVector3D vec;
                if(j >= nbVertex){
                    vec = prec;
                } else {
                    Vertex * v = f->getVertex(j);
                    vec = QVector3D(v->x(),v->y(),v->z());
                    prec = vec;
                }
                vd.position = vec;
                vd.color = f->getColor();
                vertices[i*maxValue + j] = vd;
            }
        }
    }

    // Transfer vertex data to VBO 0
    arrayBuf.bind();

    arrayBuf.allocate(vertices, nbFaces*maxValue * sizeof(VertexData));
}

void GeometryLoader::drawGeometry(QOpenGLShaderProgram *program, float alpha)
{
    // Tell OpenGL which VBOs to use
    arrayBuf.bind();

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program->attributeLocation("a_position");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(VertexData));

    int vertexColor = program->attributeLocation("a_color");
    program->enableAttributeArray(vertexColor);
    program->setAttributeBuffer(vertexColor, GL_FLOAT, sizeof(QVector3D), 1, sizeof(VertexData));

    int vertexAlpha = program->attributeLocation("a_alpha");
    program->setAttributeValue(vertexAlpha, ((alpha * 1.f) / 100.f));

    // Draw cube geometry using indices from VBO 1
    if (obj->isVEF())
        glDrawArrays(GL_QUADS, 0, obj->getNbFaces()*4);
    else
        glDrawArrays(GL_TRIANGLES, 0, obj->getNbFaces()*3);
}
