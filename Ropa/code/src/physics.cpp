#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <time.h>
#include "OurHeader.h"

#define SIZE_R 14
#define SIZE_C 18
#define SIZE 252

 bool renderSphere=false;
 bool renderCapsule = false;
 bool renderParticles = false;
 bool renderCloth=true;
 bool renderCube = false;

 bool PlaySimulation = false;

glm::vec3 Normal;

glm::vec3 pos[SIZE_C][SIZE_R];
glm::vec3 speed[SIZE_C][SIZE_R];
glm::vec3 forces[SIZE_C][SIZE_R];
glm::vec3 PointsBeforeSphereCol[SIZE_C][SIZE_R];
glm::vec3 VelocidadParticulasAnterior[SIZE_C][SIZE_R];

float distance = 0.3f;
float diagonal = glm::sqrt((glm::pow(distance, 2) + glm::pow(distance, 2)));

glm::vec3 CubePlaneNormals[6] = { glm::vec3(0.f,0.f,1.f),glm::vec3(0.f,0.f,-1.f),glm::vec3(0.f,1.f,0.f),glm::vec3(0.f,-1.f,0.f),glm::vec3(1.f,0.f,0.f),glm::vec3(-1.f,0.f,0.f) };

float elasticity = 50.f;
float damping = 2.f;
float Reset_Time=5.f;
float currentTime;
bool UseColls;
bool UseSphere;

glm::vec3 SpherePos = {0,0,0};
float SphereRadius=1.f;

glm::vec3 acceleration = { 0.f,-9.81f,0.f };
glm::vec2 k_stretch = {200,2.f};
glm::vec2 k_shear= { 200.f,2.f };
glm::vec2 k_bend= { 200.f,2.f };

glm::vec3 posBuff[SIZE_C][SIZE_R];

namespace ClothMesh {
	extern void updateClothMesh(float* array_data);
}

bool show_test_window = false;
void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, ImVec2(300, 300), 0.1f);

	
	{	
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		ImGui::Checkbox("Play simulation", &PlaySimulation);
		ImGui::InputFloat("Reset Time", &Reset_Time);
	
		ImGui::InputFloat3("Gravity", &acceleration.x);

		if (ImGui::TreeNode("Spring Parameters"))
		{
			ImGui::InputFloat2("k_stretch", &k_stretch.x);
			ImGui::InputFloat2("k_shear", &k_shear.x);
			ImGui::InputFloat2("k_bend", &k_bend.x);
			ImGui::InputFloat("Particle Link", &distance);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Collisions"))
		{
			ImGui::Checkbox("Use collisions", &UseColls);
			ImGui::Checkbox("Use Sphere Collider", &renderSphere);
		

			ImGui::TreePop();
		}
		
		
	}
	
	
	ImGui::End();

	// Example code -- ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

void SpringForcesStructural(glm::vec3 positions[][14]) 
{
	int doublei = 0;
	int doublej = 0;
	for (int i = 0; i < SIZE_C; ++i) {
		for (int j = 0; j < SIZE_R; ++j) {
			//ESQUINA IZQUIERDA ARRIBA
			if (i == SIZE_C - 1 && j ==0) {
				//STRUCTURAL
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i - 1][j]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i - 1][j]), glm::normalize((positions[i][j] - positions[i - 1][j]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j]));
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i][j + 1]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i][j + 1]), glm::normalize((positions[i][j] - positions[i][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i][j + 1]));
				//SHEAR
				forces[i][j] += -((k_shear.x*(glm::distance(positions[i][j], positions[i - 1][j + 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i - 1][j + 1]), glm::normalize((positions[i][j] - positions[i - 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j + 1]));
				//BENDING
				forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i][j + 2]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i][j + 2]), glm::normalize((positions[i][j] - positions[i][j + 2]))))*
					glm::normalize((positions[i][j] - positions[i][j + 2]));
				forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i - 2][j]) - distance*2)) + k_bend.y*glm::dot((speed[i][j] - speed[i - 2][j]), glm::normalize((positions[i][j] - positions[i - 2][j]))))*
					glm::normalize((positions[i][j] - positions[i - 2][j]));
			}
			//ESQUINA DERECHA ARRIBA
			else if (i == SIZE_C-1 && j == SIZE_R-1) {
				//STRUCTURAL
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i][j - 1]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i][j - 1]), glm::normalize((positions[i][j] - positions[i][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i][j - 1]));
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i - 1][j]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i - 1][j]), glm::normalize((positions[i][j] - positions[i - 1][j]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j]));
				//SHEAR
				forces[i][j] += -((k_shear.x*(glm::distance(positions[i][j], positions[i - 1][j - 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i - 1][j - 1]), glm::normalize((positions[i][j] - positions[i - 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j - 1]));
				//BENDING
				forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i - 2][j]) - distance*2)) + k_bend.y*glm::dot((speed[i][j] - speed[i - 2][j]), glm::normalize((positions[i][j] - positions[i - 2][j]))))*
					glm::normalize((positions[i][j] - positions[i - 2][j]));
				forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i][j - 2]) - distance*2)) + k_bend.y*glm::dot((speed[i][j] - speed[i][j - 2]), glm::normalize((positions[i][j] - positions[i][j - 2]))))*
					glm::normalize((positions[i][j] - positions[i][j - 2]));
			}
			//ESQUINA IZQUIERDA ABAJO
			else if (i == 0 && j == 0) {
				//STRUCTURAL
			
				forces[i][j] += -((k_stretch.x*(glm::length(positions[i][j]-positions[i + 1][j]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i + 1][j]), glm::normalize((positions[i][j] - positions[i + 1][j]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j]));
				forces[i][j] += -((k_stretch.x*(glm::length(positions[i][j]- positions[i][j + 1]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i][j + 1]), glm::normalize((positions[i][j] - positions[i][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i][j + 1]));
				//SHEAR
				forces[i][j] += -((k_shear.x*(glm::length(positions[i][j]- positions[i + 1][j + 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i + 1][j + 1]), glm::normalize((positions[i][j] - positions[i + 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j + 1]));
				//BENDING
				forces[i][j] += -((k_bend.x*(glm::length(positions[i][j]- positions[i + 2][j]) - distance*2)) + k_bend.y*glm::dot((speed[i][j] - speed[i + 2][j]), glm::normalize((positions[i][j] - positions[i + 2][j]))))*
					glm::normalize((positions[i][j] - positions[i + 2][j]));
				forces[i][j] += -((k_bend.x*(glm::length(positions[i][j]- positions[i][j + 2]) - distance*2)) + k_bend.y*glm::dot((speed[i][j] - speed[i][j + 2]), glm::normalize((positions[i][j] - positions[i][j + 2]))))*
					glm::normalize((positions[i][j] - positions[i][j + 2]));
			}
			//ESQUINA DERECHA ABAJO
			else if (i == 0 && j == SIZE_R-1) {
				//STRUCTURAL
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i + 1][j]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i + 1][j]), glm::normalize((positions[i][j] - positions[i + 1][j]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j]));
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i][j - 1]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i][j - 1]), glm::normalize((positions[i][j] - positions[i][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i][j - 1]));
				//SHEAR
				forces[i][j] += -((k_shear.x*(glm::distance(positions[i][j], positions[i + 1][j - 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i + 1][j - 1]), glm::normalize((positions[i][j] - positions[i + 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j - 1]));
				//BENDING
				forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i + 2][j]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i + 2][j]), glm::normalize((positions[i][j] - positions[i + 2][j]))))*
					glm::normalize((positions[i][j] - positions[i + 2][j]));
				forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i][j - 2]) - distance*2)) + k_bend.y*glm::dot((speed[i][j] - speed[i][j - 2]), glm::normalize((positions[i][j] - positions[i][j - 2]))))*
					glm::normalize((positions[i][j] - positions[i][j - 2]));
			}
			//IZQUIERDA
			else if ((1<=i)&&(i<SIZE_C-1)&&(j==0)) {
				
				//STRUCTURAL
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i + 1][j]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i + 1][j]), glm::normalize((positions[i][j] - positions[i + 1][j]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j]));
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i - 1][j]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i - 1][j]), glm::normalize((positions[i][j] - positions[i - 1][j]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j]));
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i][j + 1]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i][j + 1]), glm::normalize((positions[i][j] - positions[i][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i][j + 1]));
				//SHEAR
				forces[i][j] += -((k_shear.x*(glm::distance(positions[i][j], positions[i + 1][j + 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i + 1][j + 1]), glm::normalize((positions[i][j] - positions[i + 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j + 1]));
				forces[i][j] += -((k_shear.x*(glm::distance(positions[i][j], positions[i - 1][j + 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i - 1][j + 1]), glm::normalize((positions[i][j] - positions[i - 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j + 1]));
				//BENDING
				if (i < SIZE_C - 2 && i>1)
				{
					forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i + 2][j]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i + 2][j]), glm::normalize((positions[i][j] - positions[i + 2][j]))))*
						glm::normalize((positions[i][j] - positions[i + 2][j]));
					forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i - 2][j]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i - 2][j]), glm::normalize((positions[i][j] - positions[i - 2][j]))))*
						glm::normalize((positions[i][j] - positions[i - 2][j]));

				}
				forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i][j + 2]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i][j + 2]), glm::normalize((positions[i][j] - positions[i][j + 2]))))*
					glm::normalize((positions[i][j] - positions[i][j + 2]));
				
			}
			//DERECHA
			else if ((1<=i)&&(i<SIZE_C-1) && j == SIZE_R-1) {
				//STRUCTURAL
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i + 1][j]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i + 1][j]), glm::normalize((positions[i][j] - positions[i + 1][j]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j]));
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i - 1][j]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i - 1][j]), glm::normalize((positions[i][j] - positions[i - 1][j]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j]));
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i][j - 1]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i][j - 1]), glm::normalize((positions[i][j] - positions[i][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i][j - 1]));
				////SHEAR
				forces[i][j] += -((k_shear.x*(glm::distance(positions[i][j], positions[i + 1][j - 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i + 1][j - 1]), glm::normalize((positions[i][j] - positions[i + 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j - 1]));
				forces[i][j] += -((k_shear.x*(glm::distance(positions[i][j], positions[i - 1][j - 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i - 1][j - 1]), glm::normalize((positions[i][j] - positions[i - 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j - 1]));
				////BENDING
				if (i < SIZE_C - 2 && i>1)
				{
					forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i + 2][j]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i + 2][j]), glm::normalize((positions[i][j] - positions[i + 2][j]))))*
						glm::normalize((positions[i][j] - positions[i + 2][j]));
					forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i - 2][j]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i - 2][j]), glm::normalize((positions[i][j] - positions[i - 2][j]))))*
						glm::normalize((positions[i][j] - positions[i - 2][j]));
				}			
				forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i][j - 2]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i][j - 2]), glm::normalize((positions[i][j] - positions[i][j - 2]))))*
					glm::normalize((positions[i][j] - positions[i][j - 2]));
			}
			//ABAJO
			if (i==0&&(1<=j)&&(j<SIZE_R-1)) {
				//STRUCTURAL
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i + 1][j]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i + 1][j]), glm::normalize((positions[i][j] - positions[i + 1][j]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j]));
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i][j - 1]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i][j - 1]), glm::normalize((positions[i][j] - positions[i][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i][j - 1]));
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i][j + 1]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i][j + 1]), glm::normalize((positions[i][j] - positions[i][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i][j + 1]));
				//SHEAR
				forces[i][j] += -((k_shear.x*(glm::distance(positions[i][j], positions[i + 1][j - 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i + 1][j - 1]), glm::normalize((positions[i][j] - positions[i + 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j - 1]));
				forces[i][j] += -((k_shear.x*(glm::distance(positions[i][j], positions[i + 1][j + 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i + 1][j + 1]), glm::normalize((positions[i][j] - positions[i + 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j + 1]));
				//BENDING
				if (j < SIZE_R - 2 && j > 1)
				{
					forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i][j - 2]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i][j - 2]), glm::normalize((positions[i][j] - positions[i][j - 2]))))*
						glm::normalize((positions[i][j] - positions[i][j - 2]));
	
					forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i][j + 2]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i][j + 2]), glm::normalize((positions[i][j] - positions[i][j + 2]))))*
						glm::normalize((positions[i][j] - positions[i][j + 2]));
				}
				forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i + 2][j]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i + 2][j]), glm::normalize((positions[i][j] - positions[i + 2][j]))))*
					glm::normalize((positions[i][j] - positions[i + 2][j]));

				
			}
			//ARRIBA
			else if ((i==SIZE_C-1)&&(j<SIZE_R - 1)&&(j>=1)) {
				//STRUCTURAL
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i - 1][j]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i - 1][j]), glm::normalize((positions[i][j] - positions[i - 1][j]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j]));
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i][j - 1]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i][j - 1]), glm::normalize((positions[i][j] - positions[i][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i][j - 1]));
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i][j + 1]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i][j + 1]), glm::normalize((positions[i][j] - positions[i][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i][j + 1]));
				//SHEAR
				forces[i][j] += -((k_shear.x*(glm::distance(positions[i][j], positions[i - 1][j - 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i - 1][j - 1]), glm::normalize((positions[i][j] - positions[i - 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j - 1]));
				forces[i][j] += -((k_shear.x*(glm::distance(positions[i][j], positions[i - 1][j + 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i - 1][j + 1]), glm::normalize((positions[i][j] - positions[i - 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j + 1]));
				//BENDING
				if (j < SIZE_R - 2 && j > 1)
				{
					forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i][j + 2]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i][j + 2]), glm::normalize((positions[i][j] - positions[i][j + 2]))))*
						glm::normalize((positions[i][j] - positions[i][j + 2]));
					forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i][j - 2]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i][j - 2]), glm::normalize((positions[i][j] - positions[i][j - 2]))))*
						glm::normalize((positions[i][j] - positions[i][j - 2]));
				}
				forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i - 2][j]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i - 2][j]), glm::normalize((positions[i][j] - positions[i - 2][j]))))*
					glm::normalize((positions[i][j] - positions[i - 2][j]));
				
			}
			//RESTO
			else if (i>=1 && i<SIZE_C-1 && j>=1 && j<SIZE_R-1){
				//STRUCTURAL
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i + 1][j]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i + 1][j]), glm::normalize((positions[i][j] - positions[i + 1][j]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j]));
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i - 1][j]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i - 1][j]), glm::normalize((positions[i][j] - positions[i - 1][j]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j]));
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i][j + 1])- distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i][j + 1]), glm::normalize((positions[i][j] - positions[i][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i][j + 1]));
				forces[i][j] += -((k_stretch.x*(glm::distance(positions[i][j], positions[i][j - 1]) - distance)) + k_stretch.y*glm::dot((speed[i][j] - speed[i][j - 1]), glm::normalize((positions[i][j] - positions[i][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i][j - 1]));
				//SHEAR
				forces[i][j] += -((k_shear.x*(glm::distance(positions[i][j], positions[i + 1][j + 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i + 1][j + 1]), glm::normalize((positions[i][j] - positions[i + 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j + 1]));
				forces[i][j] += -((k_shear.x*(glm::distance(positions[i][j], positions[i - 1][j - 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i - 1][j - 1]), glm::normalize((positions[i][j] - positions[i - 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j - 1]));
				forces[i][j] += -((k_shear.x*(glm::distance(positions[i][j], positions[i - 1][j + 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i - 1][j + 1]), glm::normalize((positions[i][j] - positions[i - 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j + 1]));
				forces[i][j] += -((k_shear.x*(glm::distance(positions[i][j], positions[i + 1][j - 1]) - diagonal)) + k_shear.y*glm::dot((speed[i][j] - speed[i + 1][j - 1]), glm::normalize((positions[i][j] - positions[i + 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j - 1]));
				//BENDING
				if (j < SIZE_R - 2 && j > 1 && i < SIZE_C - 2 && i > 1)
				{
					forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i + 2][j]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i + 2][j]), glm::normalize((positions[i][j] - positions[i + 2][j]))))*
						glm::normalize((positions[i][j] - positions[i + 2][j]));
					forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i - 2][j]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i - 2][j]), glm::normalize((positions[i][j] - positions[i - 2][j]))))*
						glm::normalize((positions[i][j] - positions[i - 2][j]));
					forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i][j + 2]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i][j + 2]), glm::normalize((positions[i][j] - positions[i][j + 2]))))*
						glm::normalize((positions[i][j] - positions[i][j + 2]));
					forces[i][j] += -((k_bend.x*(glm::distance(positions[i][j], positions[i][j - 2]) - distance * 2)) + k_bend.y*glm::dot((speed[i][j] - speed[i][j - 2]), glm::normalize((positions[i][j] - positions[i][j - 2]))))*
						glm::normalize((positions[i][j] - positions[i][j - 2]));
				}
			}
		}
	}
}

void StartMesh(glm::vec3 positions[][14]) {
	for (int i = 0; i < SIZE_C; ++i) {
		for (int j = 0; j < SIZE_R; ++j) {
			posBuff[i][j] = pos[i][j] = { -2+ i*distance, 8.f, -2+j*distance };
		}
	}
}

bool CalcCollision(glm::vec3 positions[][14], glm::vec3 positionsBuff[][14],int i,int j) {
			if (((glm::dot(CubePlaneNormals[0], positions[i][j]) + 5.f)*(glm::dot(CubePlaneNormals[0], positionsBuff[i][j]) + 5.f)) <= 0.f)
			{
				Normal = CubePlaneNormals[0];

				return true;

			}
			if (((glm::dot(CubePlaneNormals[1], positions[i][j]) + 5.f)*(glm::dot(CubePlaneNormals[1], positionsBuff[i][j]) + 5.f)) <= 0.f)
			{
				Normal = CubePlaneNormals[1];

				return true;
			}
			if (((glm::dot(CubePlaneNormals[2], positions[i][j]) + 0.f)*(glm::dot(CubePlaneNormals[2], positionsBuff[i][j]) + 0.f)) <= 0.f)
			{
				Normal = CubePlaneNormals[2];

				return true;
			}
			if (((glm::dot(CubePlaneNormals[3], positions[i][j]) + 10.f)*(glm::dot(CubePlaneNormals[3], positionsBuff[i][j]) + 10.f)) <= 0.f)
			{
				Normal = CubePlaneNormals[3];

				return true;
			}
			if (((glm::dot(CubePlaneNormals[4], positions[i][j]) + 5.f)*(glm::dot(CubePlaneNormals[4], positionsBuff[i][j]) + 5.f)) <= 0.f)
			{
				Normal = CubePlaneNormals[4];

				return true;
			}
			if (((glm::dot(CubePlaneNormals[5],positions[i][j]) + 5.f)*(glm::dot(CubePlaneNormals[5], positionsBuff[i][j]) + 5.f)) <= 0.f)
			{
				Normal = CubePlaneNormals[5];
				return true;
			}
}

void CalcSphereCollision(glm::vec3 PosicionesParticulas[][SIZE_R], glm::vec3 PosicionesParticulas_2[][SIZE_R], int i, int j)
{
	glm::vec3 SphereMinusParticle;
	float DistancePartSphere;

	SphereMinusParticle = SpherePos - PosicionesParticulas[i][j];
	DistancePartSphere = (glm::sqrt((SphereMinusParticle.x*SphereMinusParticle.x) + (SphereMinusParticle.y*SphereMinusParticle.y) + (SphereMinusParticle.z*SphereMinusParticle.z))) - SphereRadius;
	
	PointsBeforeSphereCol[i][j] = PosicionesParticulas[i][j];

	if (DistancePartSphere <= 0.0f)
	{
		glm::vec3 HitPoint; //punto de choque esfera particula
		glm::vec3 VectorRecta; //vector de la recta que intersecciona con la espera, el cual es "PosicionesParticulas[i] - PointsBeforeSphereCol[i]"
		glm::vec3 NormalPlaneCollisionPoint; //normal del plano tangente al punto de colision, punto de colision menos el centro de la esfera
		glm::vec3 NewPointAfterCol;
		float Alfa;
		float Alfa1;
		float DistanciaPuntoColisionConPlano; //puntode antes de entrar en la esfera con el plano
		float DPlano; //D de la ecuación del plano, normal plano por punto de colision

		VectorRecta =PosicionesParticulas[i][j] - PointsBeforeSphereCol[i][j];

		float a = glm::pow(PointsBeforeSphereCol[i][j].x - SpherePos[0], 2) + glm::pow(PointsBeforeSphereCol[i][j].y - SpherePos[1], 2) + std::pow(PointsBeforeSphereCol[i][j].z - SpherePos[2], 2) - std::pow(SphereRadius, 2) + 2 * ((PointsBeforeSphereCol[i][j].x - SpherePos[0]) + (PointsBeforeSphereCol[i][j].y - SpherePos[1]) + (PointsBeforeSphereCol[i][j].z - SpherePos[2]));

		float b = 2.f*((PosicionesParticulas[i][j].x - PointsBeforeSphereCol[i][j].x) + (PosicionesParticulas[i][j].y - PointsBeforeSphereCol[i][j].y) + (PosicionesParticulas[i][j].z - PointsBeforeSphereCol[i][j].z)*-1.f);

		float c = std::pow(std::pow(PosicionesParticulas[i][j].x - PointsBeforeSphereCol[i][j].x, 2) + std::pow(PosicionesParticulas[i][j].y - PointsBeforeSphereCol[i][j].y, 2) + std::pow(PosicionesParticulas[i][j].z - PointsBeforeSphereCol[i][j].z, 2), 2);


		Alfa = -b + std::sqrt(std::pow(b, 2) - 4 * (a * c)) / (2 * a);
		Alfa1 = -b - std::sqrt(std::pow(b, 2) - 4 * (a * c)) / (2 * a);

		if (Alfa < Alfa1) //calculamos el punto de colision
		{
			HitPoint.x = PosicionesParticulas[i][j].x + VectorRecta.x*(Alfa);
			HitPoint.y = PosicionesParticulas[i][j].y + VectorRecta.y*(Alfa);
			HitPoint.z = PosicionesParticulas[i][j].z + VectorRecta.z*(Alfa);

			HitPoint.x = PosicionesParticulas_2[i][j].x + VectorRecta.x*(Alfa);
			HitPoint.y = PosicionesParticulas_2[i][j].y + VectorRecta.y*(Alfa);
			HitPoint.z = PosicionesParticulas_2[i][j].z + VectorRecta.z*(Alfa);
		}
		else
		{
			HitPoint.x = PosicionesParticulas[i][j].x + VectorRecta.x*(Alfa1);
			HitPoint.y = PosicionesParticulas[i][j].y + VectorRecta.y*(Alfa1);
			HitPoint.z = PosicionesParticulas[i][j].z + VectorRecta.z*(Alfa1);

			HitPoint.x = PosicionesParticulas_2[i][j].x + VectorRecta.x*(Alfa1);
			HitPoint.y = PosicionesParticulas_2[i][j].y + VectorRecta.y*(Alfa1);
			HitPoint.z = PosicionesParticulas_2[i][j].z + VectorRecta.z*(Alfa1);
		}

		NormalPlaneCollisionPoint = glm::normalize(HitPoint - SpherePos);

		DPlano = -glm::dot(NormalPlaneCollisionPoint, HitPoint);

		speed[i][j] = VelocidadParticulasAnterior[i][j];

		NewPointAfterCol = (PosicionesParticulas[i][j]) - 2.0f* (glm::dot(NormalPlaneCollisionPoint, PosicionesParticulas[i][j]) + DPlano)*NormalPlaneCollisionPoint;
		PosicionesParticulas[i][j] = NewPointAfterCol;

		speed[i][j] = VelocidadParticulasAnterior[i][j] - (2.f*(glm::dot(NormalPlaneCollisionPoint, VelocidadParticulasAnterior[i][j])) * NormalPlaneCollisionPoint);


	}


}

void CalcPosition(glm::vec3 &position, glm::vec3 &positionsBuff,glm::vec3 &speed) {
		if (Normal == glm::vec3(0.f, 1.f, 0.f)) {
			position = position - (2 * (glm::dot(Normal, position) + 0.f)*Normal);
			positionsBuff = positionsBuff - (2 * (glm::dot(Normal, positionsBuff) + 0.f)*Normal);
			speed = speed - (2 * (glm::dot(Normal, speed))*Normal);
		}

		if (Normal == glm::vec3(0.f, -1.f, 0.f)) {
			position = position - (2 * (glm::dot(Normal, position) + 10.f)*Normal);
			positionsBuff = positionsBuff - (2 * (glm::dot(Normal, positionsBuff) + 10.f)*Normal);
			speed = speed - (2 * (glm::dot(Normal, speed))*Normal);
		}

		if (Normal == glm::vec3(1.f, 0.f, 0.f)) {
			position = position - (2 * (glm::dot(Normal, position) + 5.f)*Normal);
			positionsBuff = positionsBuff - (2 * (glm::dot(Normal, positionsBuff) + 5.f)*Normal);
			speed = speed - (2 * (glm::dot(Normal, speed))*Normal);
		}

		if (Normal == glm::vec3(-1.f, 0.f, 0.f)) {
			position = position - (2 * (glm::dot(Normal, position) + 5.f)*Normal);
			positionsBuff = positionsBuff - (2 * (glm::dot(Normal, positionsBuff) + 5.f)*Normal);
			speed = speed - (2 * (glm::dot(Normal, speed))*Normal);
		}

		if (Normal == glm::vec3(0.f, 0.f, 1.f)) {
			position = position - (2 * (glm::dot(Normal, position) + 5.f)*Normal);
			positionsBuff = positionsBuff - (2 * (glm::dot(Normal, positionsBuff) + 5.f)*Normal);
			speed = speed - (2 * (glm::dot(Normal, speed))*Normal);
		}

		if (Normal == glm::vec3(0.f, 0.f, -1.f)) {
			position = position - (2 * (glm::dot(Normal, position) + 5.f)*Normal);
			positionsBuff = positionsBuff - (2 * (glm::dot(Normal, positionsBuff) + 5.f)*Normal);
			speed = speed - (2 * (glm::dot(Normal, speed))*Normal);
		}
}

void Verlet(glm::vec3 array[][14], glm::vec3 arraybuff[][14], float deltaTime) {
	glm::vec3 tmp[SIZE_C][SIZE_R];

	//RESET FORCES
	for(int i = 0; i < SIZE_C; ++i) {
		for (int j = 0; j < SIZE_R; ++j) {
			forces[i][j] = acceleration;
		}
	}
	//APPLY FORCES
	SpringForcesStructural(pos);

	//RESTRICTIONS
	diagonal = glm::sqrt((glm::pow(distance, 2) + glm::pow(distance, 2)));
	for (int i = 0; i < SIZE_C; ++i) {

		for (int j = 0; j < SIZE_R; ++j) {
			
			
			if ((i == 17&&j ==13) || (i == 17 && j == 0)) {
				
			}
			else {
				
				tmp[i][j] = array[i][j] + (array[i][j] - arraybuff[i][j]) + forces[i][j] * glm::pow(deltaTime, 2.f);
				arraybuff[i][j] = array[i][j];
				array[i][j] = tmp[i][j];
				speed[i][j] = (array[i][j] - arraybuff[i][j]) / deltaTime;
				if (CalcCollision(array, arraybuff, i, j) == true) {
					CalcPosition(array[i][j], arraybuff[i][j], speed[i][j]);
				}
			}
			if (renderSphere)
			CalcSphereCollision(pos, posBuff, i, j);
		}
	
	}
}

void PhysicsInit() {
	// Do your initialization code here...
	StartMesh(pos);
	currentTime = Reset_Time;
	srand(time(NULL));
	// ...................................
}

void PhysicsUpdate(float dt) {
	// Do your update code here...

	if (PlaySimulation)
	{
		if (currentTime <= 0) {
			StartMesh(pos);
			currentTime = Reset_Time;
			SpherePos = { rand() % 5,rand() % (5),0 };
		}
		else {
			currentTime -= dt;
		}

		for (int i = 0; i < 10; ++i) {
			Verlet(pos, posBuff, dt/10);
		}
		

		if (renderSphere)
		{
			Sphere::updateSphere(SpherePos, 1.f);
		}

		ClothMesh::updateClothMesh(&pos[0][0].x);
	}

}

void PhysicsCleanup() 
{

}