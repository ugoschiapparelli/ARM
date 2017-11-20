#include "filemanager.h"
#include "halfedge.h"
#include "vertex.h"
#include "face.h"
#include <map>
#include <tuple>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <QDebug>
#include <QFileInfo>

class Vertex;
class Face;
class Halfedge;

std::pair<unsigned int, unsigned int>
getCoVoxel(unsigned int i, unsigned int j, int sizes[3])
{
    int z = i / (sizes[0]*sizes[1]);
    int y = (i-(z*sizes[0]*sizes[1]))/sizes[0];
    int x = i-(z*sizes[0]*sizes[1])-(y*sizes[0]);
    unsigned int k,l;
    switch (j) {
    case 0: //Front
        if (z == 0)
            l = 6;
        else
            l = 2;
        k = i - sizes[0]*sizes[1];
        break;
    case 1: //Right
        if (x == sizes[0]-1)
            l = 6;
        else
            l = 3;
        k = i + 1;
        break;
    case 2: //Back
        if (z == sizes[2]-1)
            l = 6;
        else
            l = 0;
        k = i + sizes[0]*sizes[1];
        break;
    case 3: //Left
        if (x == 0)
            l = 6;
        else
            l = 1;
        k = i - 1;
        break;
    case 4: //Bottom
        if (y == sizes[1]-1)
            l = 6;
        else
            l = 5;
        k = i + sizes[0];
        break;
    case 5: //Up
        if (y == 0)
            l = 6;
        else
            l = 4;
        k = i - sizes[0];

        break;
    default:
        l = 6;
        break;
    }
    return std::pair<unsigned int, unsigned int>(k,l);
}


Face *
createFace(unsigned int i, unsigned int j, int sizes[3], std::map<std::pair<unsigned int, unsigned int>,unsigned int> * faces, std::vector<Face *> * facesId, unsigned int * cpt)
{
    std::pair<unsigned int, unsigned int> pos(i,j);
    if (faces->find(pos) == faces->end())
    {
        int z = i / (sizes[0]*sizes[1]);
        int y = (i-(z*sizes[0]*sizes[1]))/sizes[0];
        int x = i-(z*sizes[0]*sizes[1])-(y*sizes[0]);

        //Start remove - do it in the drawing part
        x -= sizes[0] / 2;
        y -= sizes[1] / 2;
        z -= sizes[2] / 2;
        //End remove

        Vertex * v0;
        Vertex * v1;
        Vertex * v2;
        Vertex * v3;

        switch (j) {
        case 0: //Front
            v0 = new Vertex(x - .5f, y - .5f, z - .5f);
            v1 = new Vertex(x - .5f, y - .5f, z + .5f);
            v2 = new Vertex(x + .5f, y - .5f, z + .5f);
            v3 = new Vertex(x + .5f, y - .5f, z - .5f);
            break;
        case 1: //Right
            v0 = new Vertex(x + .5f, y - .5f, z - .5f);
            v1 = new Vertex(x + .5f, y + .5f, z - .5f);
            v2 = new Vertex(x + .5f, y + .5f, z + .5f);
            v3 = new Vertex(x + .5f, y - .5f, z + .5f);
            break;
        case 2: //Back
            v0 = new Vertex(x + .5f, y - .5f, z + .5f);
            v1 = new Vertex(x + .5f, y + .5f, z + .5f);
            v2 = new Vertex(x - .5f, y + .5f, z + .5f);
            v3 = new Vertex(x - .5f, y - .5f, z + .5f);
            break;
        case 3: //Left
            v0 = new Vertex(x - .5f, y - .5f, z + .5f);
            v1 = new Vertex(x - .5f, y + .5f, z + .5f);
            v2 = new Vertex(x - .5f, y + .5f, z - .5f);
            v3 = new Vertex(x - .5f, y - .5f, z - .5f);
            break;
        case 4: //Bottom
            v0 = new Vertex(x - .5f, y + .5f, z - .5f);
            v1 = new Vertex(x - .5f, y + .5f, z + .5f);
            v2 = new Vertex(x + .5f, y + .5f, z + .5f);
            v3 = new Vertex(x + .5f, y + .5f, z - .5f);
            break;
        case 5: //Up
            v0 = new Vertex(x - .5f, y - .5f, z - .5f);
            v1 = new Vertex(x + .5f, y - .5f, z - .5f);
            v2 = new Vertex(x + .5f, y - .5f, z + .5f);
            v3 = new Vertex(x - .5f, y - .5f, z + .5f);
            break;
        default:
            return nullptr;
            break;
        }

        v0->setId((*cpt)++);
        v1->setId((*cpt)++);
        v2->setId((*cpt)++);
        v3->setId((*cpt)++);

        Face * f = new Face(4,0);
        f->setVertex(0, v0);
        f->setVertex(1, v1);
        f->setVertex(2, v2);
        f->setVertex(3, v3);
        facesId->push_back(f);
        faces->insert(std::pair<std::pair<unsigned int, unsigned int>, unsigned int>(pos,facesId->size()-1));

        std::pair<unsigned int, unsigned int> kl = getCoVoxel(i,j,sizes);
        //Check that k is in the image
        if (kl.second != 6)
        {
            //Check couple k, l doesnt exist
            if (faces->find(kl) == faces->end())
            {
                //Add the face to the couple
                faces->insert(std::pair<std::pair<unsigned int, unsigned int>, unsigned int>(kl,facesId->size()-1));
            }
        }

        return f;
    }

    Face * f = facesId->at(faces->at(pos));
    return f;
}

Object *
FileManager::loadPGM3D(const char *path)
{
    //Read data
    std::ifstream file(path);
    std::string format;
    std::getline(file,format);

    assert (format.compare("PGM3D") == 0 && "Not a PGM3D.");

    int sizes[3];
    int maxValue;
    //Get sizes
    file >> sizes[0] >> sizes[1] >> sizes[2];

    //Get max value
    file >> maxValue;

    long size = sizes[0] * sizes[1] * sizes[2];
    uint8_t * data = (uint8_t*)malloc(size* sizeof(uint8_t));

    long cpt(0);
    int val;
    while (file >> val)
    {
        assert (cpt < size && "Too much values.");
        assert (val <= maxValue && "Current value is higher than expected.");

        data[cpt++] = (uint8_t)val;
    }

    std::map<std::pair<unsigned int, unsigned int>,unsigned int> facesMap;
    std::vector<Face *> facesId;
    unsigned int cptV(0);
    for (unsigned int i = 0; i < size; ++i)
    {
        for (int j = 0; j < 6; ++j)
        {
            Face * f = createFace(i,j,sizes, &facesMap, &facesId, &cptV);
            //Checker if the two are different here
            //If identical then not change color
            std::pair<unsigned int, unsigned int> kl = getCoVoxel(i,j,sizes);
            if (kl.second != 6 && data[i] != data[kl.first])
            {
                uint8_t mC = data[i];
                if (data[kl.first] > mC)
                    mC = data[kl.first];
                f->setColor(mC);
            }
            else
            {
                f->setColor(0);
            }
        }
    }

    Face ** faces = (Face **)malloc(size*6*sizeof(Face*));
    for (unsigned int i = 0; i < facesId.size(); ++i)
    {
        faces[i] = facesId[i];
    }

    Object * obj = new Object(faces,facesId.size(), 3);
    obj->setVEF(true);

    return obj;
}

double
stringToDouble(std::string s)
{
  double pre = 1;
  double res = 0;
  bool minus = false;
  bool inf = true;
  int cptInf = -1;
  int iC = 0;
  for(char& c : s) {
      iC = c - '0';
      if(iC < 0){
          if(iC == -3)
             minus = true;
          if(iC == -2) // If there are more than one digits before the decimal point
              inf = false;
      } else {
          res += iC * pre;
          pre /= 10;
          if(inf)
            cptInf++;
      }
  }
  if(minus)
      res = -res;
  if(cptInf > 0)
      res = res * (10 * cptInf);
  return res;
}


Object *
FileManager::loadOBJ(const char *path, int *maxValue)
{
    //Read data
    std::ifstream file(path);

    std::vector<Vertex *> vertices;
    std::vector<Vertex *> normals;
    std::vector<Face *> faces;
    std::string line;
    while (std::getline(file,line))
    {
        std::istringstream iss(line);

        std::string key;
        std::getline( iss, key, ' ' );

        //Vertex
        if (key.compare("v") == 0)
        {
            std::string str_x, str_y, str_z;
            double x,y,z;

            std::getline( iss, str_x, ' ' );
            x = stringToDouble(str_x);

            std::getline( iss, str_y, ' ' );
            y = stringToDouble(str_y);

            std::getline( iss, str_z, ' ' );
            z = stringToDouble(str_z);

            Vertex * v = new Vertex(x,y,z);
            v->setId(vertices.size()-1);
            vertices.push_back(v);
        }

        //Normal
        if (key.compare("vn") == 0)
        {
            std::string str_x, str_y, str_z;
            double x,y,z;

            std::getline( iss, str_x, ' ' );
            x = stringToDouble(str_x);

            std::getline( iss, str_y, ' ' );
            y = stringToDouble(str_y);

            std::getline( iss, str_z, ' ' );
            z = stringToDouble(str_z);

            Vertex * n = new Vertex(x,y,z);
            n->setId(normals.size()-1);
            normals.push_back(n);
        }

        //Face
        if (key.compare("f") == 0)
        {
            std::string str_f;
            std::vector<unsigned long> idsV;
            std::vector<unsigned long> idsN;

            int nbVertex = 0;

            while(std::getline( iss, str_f, ' ' ))
            {
                std::istringstream iss_str_f(str_f);
                std::string str_id;
                std::getline( iss_str_f, str_id, '/' );
                idsV.push_back(std::stol(str_id)-1);
                std::getline( iss_str_f, str_id, '/' ); //We doesnt use Vt
                std::getline( iss_str_f, str_id, '/' );
                idsN.push_back(std::stol(str_id)-1);
                nbVertex++;
            }

            if(nbVertex > *maxValue)
                *maxValue = nbVertex;

            Face *f = new Face(nbVertex, 255);

            for (int i = 0; i < nbVertex; ++i)
            {
                f->setVertex(i, vertices[idsV.at(i)]);
            }

            for (int i = 0; i < nbVertex; ++i)
            {
                if (idsN.at(i) < normals.size())
                    f->setNormal(i,normals[idsN.at(i)]);
            }

            faces.push_back(f);
        }

    }


    //Create halfedges
    std::map<std::pair<unsigned int, unsigned int>, Halfedge *> edges;

    //Go to vertices
    for (std::vector<Face *>::iterator it = faces.begin() ; it != faces.end(); ++it)
    {
        Face *f = *it;

        int nbVertex = f->getNbVertex();
        for (int i=0; i < nbVertex; i++)
        {
            int j = i+1;
            if (i == nbVertex-1)
                j = 0;

            Vertex * u = f->getVertex(i);
            Vertex * v = f->getVertex(j);
            std::pair<unsigned int, unsigned int> pos(u->id(),v->id());
            Halfedge * h = new Halfedge();
            h->setFace(f);
            h->setVertex(v);
            edges[pos] = h;
        }

        //Set next and opposite
        for (int i=0; i < nbVertex; i++)
        {
            int j = i+1;
            if (j == nbVertex)
                j = 0;

            int k = j+1;
            if (k == nbVertex)
                k = 0;

            Vertex * u = f->getVertex(i);
            Vertex * v = f->getVertex(j);
            Vertex * w = f->getVertex(k);
            std::pair<unsigned int, unsigned int> pos1(u->id(),v->id());
            std::pair<unsigned int, unsigned int> pos2(v->id(),w->id());
            Halfedge * h = edges[pos1];
            Halfedge * next = edges[pos2];
            h->setNextHalfedge(next);

            std::pair<unsigned int, unsigned int> pos3(v->id(),u->id());
            if (edges.find(pos3) != edges.end())
            {
                Halfedge * opposite = edges[pos3];
                h->setOppositeHalfedge(opposite);
                opposite->setOppositeHalfedge(h);
            }
        }
    }

    //Create object
    Halfedge ** objEdges = (Halfedge**)malloc(edges.size()*sizeof(Halfedge*));
    unsigned int cptEdges(0);
    for (const auto& E : edges) {
        Halfedge *e = E.second;

        objEdges[cptEdges++] = e;
    }

    Face ** fobj = (Face **)malloc(faces.size()*sizeof(Face*));
    for (int i=0; i < faces.size(); ++i)
    {
        fobj[i] = faces.at(i);
    }

    Object * obj = new Object(fobj, faces.size(), *maxValue);

    return obj;
}


void
FileManager::saveOBJ(const char *path, Object * obj)
{
    std::ofstream fout(path);

    fout << "# Test file" << std::endl;

    Face ** faces = obj->getFaces();
    unsigned int nbFaces = obj->getNbFaces();

    std::map<unsigned int, Vertex *> vertexMap;
    std::map<unsigned int, unsigned int> realToLocalIndexMapV;
    std::map<unsigned int, unsigned int> localToRealIndexMapV;
    unsigned int cptV(1);
    std::map<unsigned int, Vertex *> normalMap;
    std::map<unsigned int, unsigned int> realToLocalIndexMapN;
    std::map<unsigned int, unsigned int> localToRealIndexMapN;
    unsigned int cptN(1);

    //Go to vertices
    for (unsigned int i = 0; i < nbFaces; i++)
    {
        Face * f = faces[i];
        int nbVertex = f->getNbVertex();
        if (f->getColor() != 0)
        {
            for (unsigned int j=0; j < nbVertex; ++j)
            {
                //Vertex
                Vertex * v = f->getVertex(j);
                if (v != nullptr)
                {
                    unsigned int id = v->id();
                    if (vertexMap.find(id) == vertexMap.end())
                    {
                        vertexMap.insert(std::pair<unsigned int, Vertex *>(id,v));
                        realToLocalIndexMapV.insert(std::pair<unsigned int, unsigned int>(cptV,id));
                        localToRealIndexMapV.insert(std::pair<unsigned int, unsigned int>(id,cptV));
                        cptV++;
                    }
                }

                //Normal
                Vertex * n = f->getNormal(j);
                if (n != nullptr)
                {
                    unsigned int id = n->id();
                    if (normalMap.find(id) == normalMap.end())
                    {
                        normalMap.insert(std::pair<unsigned int, Vertex *>(id,n));
                        realToLocalIndexMapN.insert(std::pair<unsigned int, unsigned int>(cptN,id));
                        localToRealIndexMapN.insert(std::pair<unsigned int, unsigned int>(id,cptN));
                        cptN++;
                    }
                }
            }
        }
    }

    //Writing vertices
    for (unsigned int i = 1; i < cptV; i++)
    {
        Vertex *v = vertexMap.at(realToLocalIndexMapV.at(i));
        fout << "v ";
        fout << v->x();
        fout << " ";
        fout << v->y();
        fout << " ";
        fout << v->z();
        fout << std::endl;
    }

    //Writing normals
    for (unsigned int i = 1; i < cptN; i++)
    {
        Vertex *n = normalMap.at(realToLocalIndexMapN.at(i));
        fout << "vn ";
        fout << n->x();
        fout << " ";
        fout << n->y();
        fout << " ";
        fout << n->z();
        fout << std::endl;
    }

    fout << "s off" << std::endl;

    //Writing faces
    for (unsigned int i = 0; i < nbFaces; i++)
    {
        Face * f = faces[i];
        int nbVertex = f->getNbVertex();

        if (f->getColor() != 0)
        {

            fout << "f ";

            for (int j=0; j < nbVertex; ++j)
            {
                unsigned int idV = localToRealIndexMapV.at(f->getVertex(j)->id());
                fout << idV;

                if (f->getNormal(j) != nullptr)
                {
                    unsigned int idN = localToRealIndexMapN.at(f->getNormal(j)->id());
                    fout << "//";
                    fout << idN;
                }

                fout << " ";
            }

            fout << std::endl;

        }
    }
}
