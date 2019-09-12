#include <vector>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>
#include "vec3.h"
#include "vec2.h"
#include "matrix44.h"

#define min_x 0
#define max_x 1
#define min_y 2
#define max_y 3
#define min_z 4
#define max_z 5

#define M_PI 3.141592653589793

struct Vertex{
	vec3 pos;
};

struct Triangle {

	Vertex vertex[3];

	Triangle( const std::vector<vec3> v )
	{
		vertex[0].pos = v[0];
		vertex[1].pos = v[1];
		vertex[2].pos = v[2];
	}

	~Triangle(){}
};

class Mesh 
{
public:
	std::vector<Triangle> tris;

	Mesh() {}
	~Mesh() {}

	bool load_mesh_from_file(const char* path) 
	{
		tris.clear();
		std::vector< unsigned int > vertexIndices;
		std::vector< vec3 > temp_vertices;

		std::ifstream f(path);
		if (!f.is_open())
		{
			std::cout << "File cannot be oppened or does not exist\n";
			return false;
		}

		std::cout << "file was  oppened!\n";

		
		while (!f.eof())
		{
			char line[256];
			f.getline(line, 256);

			std::stringstream s;
			s << line;

			char junk;

			if ( line[0] == 'v')
			{
				if (line[1] == 't') 
				{
				}
				else if (line[1] == 'n') 
				{
				}
				else {
					vec3 vertex;
					s >> junk >> vertex[0] >> vertex[1] >> vertex[2];
					temp_vertices.push_back(vertex);
				}
			}

			else if ( line[0] == 'f')
			{
				std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3];

				s >> junk >> vertex1 >> vertex2 >> vertex3;
				int fstslash = vertex1.find("/");
				std::string fst = vertex1.substr(0, fstslash);
				vertexIndex[0] = atoi( fst.c_str() );

				fstslash = vertex2.find("/");
				fst = vertex2.substr(0, fstslash);
				vertexIndex[1] = atoi(fst.c_str());

				fstslash = vertex3.find("/");
				fst = vertex3.substr(0, fstslash);
				vertexIndex[2] = atoi(fst.c_str());

				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);
			}
		}

		for (unsigned int i = 0; i < vertexIndices.size(); i+=3)
		{
			unsigned int v1 = vertexIndices[i];
			unsigned int v2 = vertexIndices[i+1];
			unsigned int v3 = vertexIndices[i+2];

			std::vector<vec3> vertices;
			vertices.push_back(temp_vertices[v1 - 1]);
			vertices.push_back(temp_vertices[v2 - 1]);
			vertices.push_back(temp_vertices[v3 - 1]);

			Triangle t(vertices);
			tris.push_back(t);
		}

		std::cout << "vertSize = " << vertexIndices.size() << "\n";
		return true;
	}
};

class Obj 
{
public:
	Mesh mesh;

	Obj(){}
	Obj( const char* file_path ){
		mesh.load_mesh_from_file(file_path); 
	}
	
	~Obj(){}
};