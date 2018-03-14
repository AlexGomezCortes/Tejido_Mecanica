#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm/glm.hpp>
#include <iostream>

#define SIZE_R 14
#define SIZE_C 18
#define SIZE 252

glm::vec3 pos[SIZE_C][SIZE_R];
glm::vec3 speed[SIZE_C][SIZE_R];
glm::vec3 forces[SIZE_C][SIZE_R];
float distance = 0.5f;
float diagonal = glm::sqrt((glm::pow(distance, 2) + glm::pow(distance, 2)));
float elasticity = 4.f;
float damping = 2.f;
float Reset_Time;
bool UseColls;
bool UseSphere;
glm::vec3 acceleration;
glm::vec2 k_stretch;
glm::vec2 k_shear;
glm::vec2 k_bend;
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
		ImGui::InputFloat3("Gravity", &acceleration.x);
		ImGui::InputFloat("Reset Time", &Reset_Time);

		if (ImGui::TreeNode("Spring Parameters"))
		{
			ImGui::InputFloat2("k_stretch", &k_stretch.x);
			ImGui::InputFloat2("k_shear", &k_shear.x);
			ImGui::InputFloat2("k_bend", &k_bend.x);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Collisions"))
		{
			ImGui::Checkbox("Use collisions", &UseColls);
			ImGui::Checkbox("Use Sphere Collider", &UseSphere);

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
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i - 1][j]), glm::normalize((positions[i][j] - positions[i - 1][j]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j + 1])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j + 1]), glm::normalize((positions[i][j] - positions[i][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i][j + 1]));
				//SHEAR
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j + 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i - 1][j + 1]), glm::normalize((positions[i][j] - positions[i - 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j + 1]));
				//BENDING
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j + 2])) - distance * 2) + glm::dot(damping*(speed[i][j] - speed[i][j + 2]), glm::normalize((positions[i][j] - positions[i][j + 2]))))*
					glm::normalize((positions[i][j] - positions[i][j + 2]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 2][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i - 2][j]), glm::normalize((positions[i][j] - positions[i - 2][j]))))*
					glm::normalize((positions[i][j] - positions[i - 2][j]));
			}
			//ESQUINA DERECHA ARRIBA
			else if (i == SIZE_C-1 && j == SIZE_R-1) {
				//STRUCTURAL
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j - 1])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j - 1]), glm::normalize((positions[i][j] - positions[i][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i][j - 1]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i - 1][j]), glm::normalize((positions[i][j] - positions[i - 1][j]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j]));
				//SHEAR
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j - 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i - 1][j - 1]), glm::normalize((positions[i][j] - positions[i - 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j - 1]));
				//BENDING
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 2][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i - 2][j]), glm::normalize((positions[i][j] - positions[i - 2][j]))))*
					glm::normalize((positions[i][j] - positions[i - 2][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j - 2])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j - 2]), glm::normalize((positions[i][j] - positions[i][j - 2]))))*
					glm::normalize((positions[i][j] - positions[i][j - 2]));
			}
			//ESQUINA IZQUIERDA ABAJO
			else if (i == 0 && j == 0) {
				//STRUCTURAL
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i + 1][j]), glm::normalize((positions[i][j] - positions[i + 1][j]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j + 1])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j + 1]), glm::normalize((positions[i][j] - positions[i][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i][j + 1]));
				//SHEAR
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j + 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i + 1][j + 1]), glm::normalize((positions[i][j] - positions[i + 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j + 1]));
				//BENDING
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 2][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i + 2][j]), glm::normalize((positions[i][j] - positions[i + 2][j]))))*
					glm::normalize((positions[i][j] - positions[i + 2][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j + 2])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j + 2]), glm::normalize((positions[i][j] - positions[i][j + 2]))))*
					glm::normalize((positions[i][j] - positions[i][j + 2]));
			}
			//ESQUINA DERECHA ABAJO
			else if (i == 0 && j == SIZE_R-1) {
				//STRUCTURAL
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i + 1][j]), glm::normalize((positions[i][j] - positions[i + 1][j]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j - 1])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j - 1]), glm::normalize((positions[i][j] - positions[i][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i][j - 1]));
				//SHEAR
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j - 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i + 1][j - 1]), glm::normalize((positions[i][j] - positions[i + 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j - 1]));
				//BENDING
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 2][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i + 2][j]), glm::normalize((positions[i][j] - positions[i + 2][j]))))*
					glm::normalize((positions[i][j] - positions[i + 2][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j - 2])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j - 2]), glm::normalize((positions[i][j] - positions[i][j - 2]))))*
					glm::normalize((positions[i][j] - positions[i][j - 2]));
			}
			//IZQUIERDA
			else if ((1<i<SIZE_C-1)&&j==0) {
				//STRUCTURAL
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i + 1][j]), glm::normalize((positions[i][j] - positions[i + 1][j]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i - 1][j]), glm::normalize((positions[i][j] - positions[i - 1][j]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j + 1])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j + 1]), glm::normalize((positions[i][j] - positions[i][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i][j + 1]));
				//SHEAR
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j + 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i + 1][j + 1]), glm::normalize((positions[i][j] - positions[i + 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j + 1]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j + 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i - 1][j + 1]), glm::normalize((positions[i][j] - positions[i - 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j + 1]));
				//BENDING
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 2][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i + 2][j]), glm::normalize((positions[i][j] - positions[i + 2][j]))))*
					glm::normalize((positions[i][j] - positions[i + 2][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 2][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i - 2][j]), glm::normalize((positions[i][j] - positions[i - 2][j]))))*
					glm::normalize((positions[i][j] - positions[i - 2][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j + 2])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j + 2]), glm::normalize((positions[i][j] - positions[i][j + 2]))))*
					glm::normalize((positions[i][j] - positions[i][j + 2]));
			}
			//DERECHA
			else if ((1<i<SIZE_C-1) && j == SIZE_R-1) {
				//STRUCTURAL
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i + 1][j]), glm::normalize((positions[i][j] - positions[i + 1][j]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i - 1][j]), glm::normalize((positions[i][j] - positions[i - 1][j]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j - 1])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j - 1]), glm::normalize((positions[i][j] - positions[i][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i][j - 1]));
				//SHEAR
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j - 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i + 1][j - 1]), glm::normalize((positions[i][j] - positions[i + 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j - 1]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j - 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i - 1][j - 1]), glm::normalize((positions[i][j] - positions[i - 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j - 1]));
				//BENDING
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 2][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i + 2][j]), glm::normalize((positions[i][j] - positions[i + 2][j]))))*
					glm::normalize((positions[i][j] - positions[i + 2][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 2][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i - 2][j]), glm::normalize((positions[i][j] - positions[i - 2][j]))))*
					glm::normalize((positions[i][j] - positions[i - 2][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j - 2])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j - 2]), glm::normalize((positions[i][j] - positions[i][j - 2]))))*
					glm::normalize((positions[i][j] - positions[i][j - 2]));
			}
			//ABAJO
			else if (i==0&&(1<j<SIZE_R-1)) {
				//STRUCTURAL
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i + 1][j]), glm::normalize((positions[i][j] - positions[i + 1][j]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j - 1])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j - 1]), glm::normalize((positions[i][j] - positions[i][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i][j - 1]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j + 1])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j + 1]), glm::normalize((positions[i][j] - positions[i][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i][j + 1]));
				//SHEAR
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j - 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i + 1][j - 1]), glm::normalize((positions[i][j] - positions[i + 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j - 1]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j + 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i + 1][j + 1]), glm::normalize((positions[i][j] - positions[i + 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j + 1]));
				//BENDING
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 2][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i + 2][j]), glm::normalize((positions[i][j] - positions[i + 2][j]))))*
					glm::normalize((positions[i][j] - positions[i + 2][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j + 2])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j + 2]), glm::normalize((positions[i][j] - positions[i][j + 2]))))*
					glm::normalize((positions[i][j] - positions[i][j + 2]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j - 2])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j - 2]), glm::normalize((positions[i][j] - positions[i][j - 2]))))*
					glm::normalize((positions[i][j] - positions[i][j - 2]));
			}
			//ARRIBA
			else if (i==SIZE_C-1&&(SIZE_R - 1>j>1)) {
				//STRUCTURAL
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i + 1][j]), glm::normalize((positions[i][j] - positions[i + 1][j]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i - 1][j]), glm::normalize((positions[i][j] - positions[i - 1][j]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j - 1])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j - 1]), glm::normalize((positions[i][j] - positions[i][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i][j - 1]));
				//SHEAR
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j - 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i - 1][j - 1]), glm::normalize((positions[i][j] - positions[i - 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j - 1]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j + 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i - 1][j + 1]), glm::normalize((positions[i][j] - positions[i - 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j + 1]));
				//BENDING
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 2][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i - 2][j]), glm::normalize((positions[i][j] - positions[i - 2][j]))))*
					glm::normalize((positions[i][j] - positions[i - 2][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j + 2])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j + 2]), glm::normalize((positions[i][j] - positions[i][j + 2]))))*
					glm::normalize((positions[i][j] - positions[i][j + 2]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j - 2])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j - 2]), glm::normalize((positions[i][j] - positions[i][j - 2]))))*
					glm::normalize((positions[i][j] - positions[i][j - 2]));
			}
			//RESTO
			else{
				//STRUCTURAL
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i + 1][j]), glm::normalize((positions[i][j] - positions[i + 1][j]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i - 1][j]), glm::normalize((positions[i][j] - positions[i - 1][j]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j + 1])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j + 1]), glm::normalize((positions[i][j] - positions[i][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i][j + 1]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j - 1])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j - 1]), glm::normalize((positions[i][j] - positions[i][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i][j - 1]));
				//SHEAR
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j + 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i + 1][j + 1]), glm::normalize((positions[i][j] - positions[i + 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j + 1]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j - 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i - 1][j - 1]), glm::normalize((positions[i][j] - positions[i - 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j - 1]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j + 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i - 1][j + 1]), glm::normalize((positions[i][j] - positions[i - 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j + 1]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j - 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i + 1][j - 1]), glm::normalize((positions[i][j] - positions[i + 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j - 1]));
				//BENDING
<<<<<<< HEAD
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 2][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i + 2][j]), glm::normalize((positions[i][j] - positions[i + 2][j]))))*
					glm::normalize((positions[i][j] - positions[i + 2][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 2][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i - 2][j]), glm::normalize((positions[i][j] - positions[i - 2][j]))))*
					glm::normalize((positions[i][j] - positions[i - 2][j]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j + 2])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j + 2]), glm::normalize((positions[i][j] - positions[i][j + 2]))))*
					glm::normalize((positions[i][j] - positions[i][j + 2]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j - 2])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j - 2]), glm::normalize((positions[i][j] - positions[i][j - 2]))))*
					glm::normalize((positions[i][j] - positions[i][j - 2]));
				
			}
		}
	}

=======
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j + 1])) - distance * 2) + glm::dot(damping*(speed[i][j] - speed[i + 1][j + 1]), glm::normalize((positions[i][j] - positions[i + 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j + 1]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j - 1])) - distance * 2) + glm::dot(damping*(speed[i][j] - speed[i - 1][j - 1]), glm::normalize((positions[i][j] - positions[i - 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j - 1]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j + 1])) - distance * 2) + glm::dot(damping*(speed[i][j] - speed[i - 1][j + 1]), glm::normalize((positions[i][j] - positions[i - 1][j + 1]))))*
					glm::normalize((positions[i][j] - positions[i - 1][j + 1]));
				forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j - 1])) - distance * 2) + glm::dot(damping*(speed[i][j] - speed[i + 1][j - 1]), glm::normalize((positions[i][j] - positions[i + 1][j - 1]))))*
					glm::normalize((positions[i][j] - positions[i + 1][j - 1]));
			}

			}
		}
>>>>>>> master
}

void SpringForcesShear(glm::vec3 positions[][14]) {
	for (int i = 0; i < SIZE_C; ++i) {
		for (int j = 0; j < SIZE_R; ++j) {
			forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j + 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i + 1][j + 1]), glm::normalize((positions[i][j] - positions[i + 1][j + 1]))))*
				glm::normalize((positions[i][j] - positions[i + 1][j + 1]));
			forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j - 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i - 1][j - 1]), glm::normalize((positions[i][j] - positions[i - 1][j - 1]))))*
				glm::normalize((positions[i][j] - positions[i - 1][j - 1]));
			forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 1][j + 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i - 1][j + 1]), glm::normalize((positions[i][j] - positions[i - 1][j + 1]))))*
				glm::normalize((positions[i][j] - positions[i - 1][j + 1]));
			forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 1][j - 1])) - diagonal) + glm::dot(damping*(speed[i][j] - speed[i + 1][j - 1]), glm::normalize((positions[i][j] - positions[i + 1][j - 1]))))*
				glm::normalize((positions[i][j] - positions[i + 1][j - 1]));
		}
	}
}

void SpringForcesBending(glm::vec3 positions[][14]) {
	for (int i = 0; i < SIZE_C; i ++) {
		for (int j = 0; j < SIZE_R; j ++) {

			forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i + 2][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i + 2][j]), glm::normalize((positions[i][j] - positions[i + 2][j]))))*
				glm::normalize((positions[i][j] - positions[i + 2][j]));
			forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i - 2][j])) - distance) + glm::dot(damping*(speed[i][j] - speed[i - 2][j]), glm::normalize((positions[i][j] - positions[i - 2][j]))))*
				glm::normalize((positions[i][j] - positions[i - 2][j]));
			forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j + 2])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j + 2]), glm::normalize((positions[i][j] - positions[i][j + 2]))))*
				glm::normalize((positions[i][j] - positions[i][j + 2]));
			forces[i][j] += -((elasticity*(glm::distance(positions[i][j], positions[i][j - 2])) - distance) + glm::dot(damping*(speed[i][j] - speed[i][j - 2]), glm::normalize((positions[i][j] - positions[i][j - 2]))))*
				glm::normalize((positions[i][j] - positions[i][j - 2]));
		}
	}
}

void Verlet(glm::vec3 array[][14], glm::vec3 arraybuff[][14], float deltaTime) {
	glm::vec3 gravity = { 0, -9.8f, 0 };
	glm::vec3 tmp[SIZE_C][SIZE_R];
	//RESET FORCES
	for (int i = 0; i < SIZE_C; ++i) {
		for (int j = 0; j < SIZE_R; ++j) {
			forces[i][j] = gravity;
		}
	}
	//APPLY FORCES
	SpringForcesStructural(pos);
	
	//SpringForcesShear(pos);
	//SpringForcesBending(pos);

	//RESTRICTIONS
	for (int i = 0; i < SIZE_C; ++i) {
		for (int j = 0; j < SIZE_R; ++j) {
			
			if ((i != 17&&j !=13) || (i != 17 && j != 0)) {
				tmp[i][j] = array[i][j] + (array[i][j] - arraybuff[i][j]) + forces[i][j] * glm::pow(deltaTime, 2.f);
				arraybuff[i][j] = array[i][j];
				array[i][j] = tmp[i][j];
				speed[i][j] = (array[i][j] - arraybuff[i][j]) / deltaTime;
			}
			else {
				
			}
		}
	}
}

void PhysicsInit() {
	// Do your initialization code here...
	for (int i = 0; i < SIZE_C;++i) {
		for (int j = 0; j < SIZE_R; ++j) {
			/*if ((i == 17 && j == 13)) {
				posBuff[i][j] = pos[i][j] = { 3, 10, 5 };
			}
			else if ((i == 17 && j == 0)) {
				posBuff[i][j] = pos[i][j] = { 3, 10, -5 };
			}
			else {
				
			}*/
			posBuff[i][j] = pos[i][j] = { 3, i*distance, -5 + j*distance };
		}
	}
	
	// ...................................
}

void PhysicsUpdate(float dt) {
	// Do your update code here...
	Verlet(pos, posBuff, dt);

	ClothMesh::updateClothMesh(&pos[0][0].x);
	
	// ...........................
}

void PhysicsCleanup() {
	// Do your cleanup code here...
	// ............................
}