#ifndef CAMERAH
#define CAMERAH

#include "vec3.h"
#include "vec2.h"
#include "matrix44.h"
#include "object.h"

#include <SDL.h>

const int WIDTH = 600;
const int HEIGHT = 400;
const int LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8;

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
	
	int get_outcode(vec2 point) {
		int outcode = 0;
		//Se o y do ponto for maior que o Ymax (imgHeight) setar bit mais significativo para 1
		if (point.y() > imgHeight)
			outcode |= TOP;
		else if (point.y() < 0) //Caso seja menor que o Ymin, setar 2 bit mais significativo para 1
			outcode |= BOTTOM;
		//Se o x do ponto for maior que o Xmax (imgWidth) setar 3 bit mais significativo para 1
		if (point.x() > imgWidth)
			outcode |= RIGHT;
		else if (point.x() < 0) //Caso seja menor que o Xmin, setar 4 bit mais significativo para 1
			outcode |= LEFT;
		
		return outcode;
	}

	bool clip_line(vec2 &p0, vec2 &p1) {
		//Determinando outcode de p0 e p1 e inicializacao de x_max e y_max
		int outcode_p0 = get_outcode(p0), outcode_p1 = get_outcode(p1), outcode_external;
		float x_max = (float)imgWidth, y_max = (float)imgHeight;
		bool accepted = false;
		
		while (!accepted) {
			//Caso a reta esteja dentro da tela
			if (outcode_p0 == 0 || outcode_p1 == 0) {
				accepted = true;
				break;
			//Caso a reta nao passe pela tela
			} else if ((outcode_p0 & outcode_p1) != 0) {
				break;
			} else { //Casos em que a reta passa pela tela mas precisa ser cortada
				float slope, tempX = 0, tempY = 0;
				//Inclinacao da reta calculada
				slope = (p1.y() - p0.y()) / (p1.x() - p0.x());
				outcode_external = (outcode_p0 != 0 ? outcode_p0 : outcode_p1);
				//Se estiver no quadrante TOP
				if (outcode_external & TOP) {
					tempX = p0.x() + (y_max - p0.y())/slope;
					tempY = y_max;
				//Se estiver no quadrante BOTTOM
				} else if (outcode_external & BOTTOM) {
					tempX = p0.x() + (-p0.y()) / slope;
					tempY = 0;
				//Se estiver no quadrante RIGHT
				} else if (outcode_external & RIGHT) {
					tempX = x_max;
					tempY = p0.y() + (x_max - p0.x()) * slope;
				//Se estiver no quadrante LEFT
				} else if (outcode_external & LEFT) {
					tempX = 0;
					tempY = p0.y() + (-p0.x()) * slope;
				}
				//Se tivermos editado o p0, atualizar seus valores e recalcular seu outcode
				if (outcode_external == outcode_p0) {
					p0[0] = tempX;
					p0[1] = tempY;
					outcode_p0 = get_outcode(p0);
				} else { //Caso contrario editamos p1 e precisamos atualiza-lo
					p1[0] = tempX;
					p1[1] = tempY;
					outcode_p1 = get_outcode(p1);
				}
			}
		}

		return accepted;
	}

	void draw_line(SDL_Renderer* renderer, vec2 &p0, vec2 &p1) {
		//Criacao do Vetor Diretor
		vec2 director = p1 - p0;
		//Determinando limite de iteracao
		int range = (int)director.length();
		//Vetor unitario para desenhar cada ponto da reta
		director.make_unit_vector();
		//Determinando ponto inicial do vetor e em seguida iteracoes para desenhar os pontos da reta
		vec2 start_point = p0;
		for (int i = 0; i < range; ++i, start_point += director)
			SDL_RenderDrawPoint(renderer, start_point.x(), start_point.y());
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

				if (v1 && v2) {
					//Se estiver na tela, desenhar a linha
					if(clip_line(praster1, praster2))
						draw_line(renderer, praster1, praster2);
				}
				if (v1 && v3) {
					//Se estiver na tela, desenhar a linha
					if (clip_line(praster1, praster3))
						draw_line(renderer, praster1, praster3);
				}
				if (v2 && v3) {
					//Se estiver na tela, desenhar a linha
					if (clip_line(praster2, praster3))
						draw_line(renderer, praster2, praster3);
				}
            }
        }
    }
};


#endif