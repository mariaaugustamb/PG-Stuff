#ifndef CAMERAH
#define CAMERAH

#include "vec3.h"
#include "vec2.h"
#include "matrix44.h"
#include "object.h"

#include <SDL.h>

const int WIDTH = 600;
const int HEIGHT = 400;

class camera
{
public:
    int imgWidth, imgHeight;
    float fov, _near, _far;
    float bottom, left, top, right;
    matrix44 camToWorld;
    matrix44 worldToCamera;

	vec3 _from, _at, _up;
    vec3 axisX, axisY, axisZ;

public:
    camera();
    camera(const vec3 &from, const vec3 &at, const vec3 &up, const float &f, const float &n, const float &far, const int &iwidth, const int &iheight): 
           fov(f), _near(n), imgWidth(iwidth), imgHeight(iheight), _from(from), _at(at), _up(up), _far(far) {
                look_at(from, at, up);
				camToWorld = matrix44(axisX[0], axisX[1], axisX[2], 0.f,
									axisY[0], axisY[1], axisY[2], 0.f,
									axisZ[0], axisZ[1], axisZ[2], 0.f,
									_from[0], _from[1], _from[2], 1.f);
				worldToCamera = camToWorld.inverse();
				top = tan(fov / 2 * (M_PI/180));
				right = top * ((float)imgWidth / (float)imgHeight);
				left = -right;
				bottom = -top;
           }
	//dot retorna produto interno 2 vetores; cross retorna produto vetorial
    void look_at(const vec3 &from, const vec3 &at, const vec3 &up) {
		axisZ = from - at;
		axisZ.make_unit_vector();
		axisY = up - ((dot(up, axisZ) / dot(axisZ, axisZ)) * axisZ);
		axisY.make_unit_vector();
		axisX = cross(axisY, axisZ);
		axisX.make_unit_vector();
    }

    bool compute_pixel_coordinates(const vec3 &pWorld, vec2 &pRaster) {
		vec3 pCamera, pNDC, pScreen;
		matrix44 projectionMatrix, normalizationMatrix;
		projectionMatrix = matrix44(1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 1.f / _near,
			0.f, 0.f, 0.f, 1.f);
		normalizationMatrix = matrix44(2.f * _near / (right - left), 0.f, 0.f, 0.f,
			0.f, 2.f * _near / (bottom - top), 0.f, 0.f,
			(-(right + left)) / (right - left), (-(bottom + top)) / (bottom - top), (_far + _near) / (_far - _near), 1.f,
			0.f, 0.f, (-(2.f * _near)) / (_far - _near), 0.f);
		//Conversao do World Space para o Camera Space
		worldToCamera.mult_point_matrix(pWorld, pCamera);
		//Projecao do ponto do Camera Space para Screen Space
		projectionMatrix.mult_point_matrix(pCamera, pScreen);
		float dz = _near / pCamera[2];
		//Homegenizacao do ponto
		//pScreen[0] *= dz;
		//pScreen[1] *= dz;
		//pScreen[2] *= dz;
		//Normalizacao do ponto para o NDC Space
		normalizationMatrix.mult_point_matrix(pScreen, pNDC);
		if (abs(pNDC[0]) > right || abs(pNDC[1]) > top)
			return false;
		//Conversao da NDC Space para o Raster Space 
		pRaster[0] = ((1+pNDC[0]) / 2) * imgWidth;
		pRaster[1] = ((1-pNDC[1]) / 2) * imgHeight;
		//Retornar verdadeiro caso o ponto esteja no mesmo plano da imagem
		return true;
    }

    void render_scene( std::vector<Obj> objs, SDL_Renderer* renderer) {

        vec3 light(0.0f, 0.0f, -1.0f);
        light.make_unit_vector();

        for (auto obj : objs){
            for (int i = 0; i < obj.mesh.tris.size(); i++)
            {
                vec2 praster1;
                vec2 praster2;
                vec2 praster3;

                vec3 col(255, 255, 255);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

                bool v1, v2, v3;
                v1 = compute_pixel_coordinates(obj.mesh.tris[i].vertex[0].pos, praster1);
                v2 = compute_pixel_coordinates(obj.mesh.tris[i].vertex[1].pos, praster2);
                v3 = compute_pixel_coordinates(obj.mesh.tris[i].vertex[2].pos, praster3);

                if(v1 && v2)
                    SDL_RenderDrawLine(renderer, praster1.x(), praster1.y(), praster2.x(), praster2.y());
                if(v1 && v3)
                    SDL_RenderDrawLine(renderer, praster1.x(), praster1.y(), praster3.x(), praster3.y());
                if(v2 && v3)
                    SDL_RenderDrawLine(renderer, praster2.x(), praster2.y(), praster3.x(), praster3.y());
            }
        }
    }
};


#endif