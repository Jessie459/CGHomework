#pragma once
#include "shader.h"
#include <vector>
#include <cstdlib>
#include <ctime>

typedef struct {
	glm::vec3 position;
	glm::vec3 velosity;
	glm::vec3 acceleration;
	glm::vec4 color;
	GLfloat size;
	GLfloat life;
} Particle;

class ParticleGenerator {
public:
	ParticleGenerator(GLuint amount) {
		this->amount = amount;
		this->init();
	}

	void Update(GLfloat dt, GLuint newParticles) {
		// ���������
		for (GLuint i = 0; i < newParticles; i++) {
			int unusedParticle = this->firstUnusedParticle();
			this->respawnParticle(this->particles[unusedParticle]);
		}

		// ������������
		for (GLuint i = 0; i < this->amount; i++) {
			Particle &p = this->particles[i];
			// ������������
			p.life -= dt;
			if (p.life > 0.0f) {
				// ��������λ��
				p.position += p.velosity * dt + 0.5f * p.acceleration * dt * dt;
				// ���������ٶ�
				p.velosity += p.acceleration * dt;
			}
		}
	}

	void Draw(Shader shader, GLuint textureID) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		for (Particle particle : this->particles) {
			if (particle.life > 0) {
				shader.use();
				shader.setVec3("offset", particle.position);
				shader.setVec4("color", particle.color);
				shader.setFloat("size", particle.size);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textureID);
				glBindVertexArray(this->VAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		}
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

private:
	std::vector<Particle> particles;
	GLuint amount;
	GLuint VAO, VBO;

	void init() {
		GLfloat particle_quad[] = {
			// λ������          // ��������
			-0.1f,  0.1f, 0.0f,  0.0f, 1.0f,
			 0.1f, -0.1f, 0.0f,  1.0f, 0.0f,
			-0.1f, -0.1f, 0.0f,  0.0f, 0.0f,

			-0.1f,  0.1f, 0.0f,  0.0f, 1.0f,
			 0.1f,  0.1f, 0.0f,  1.0f, 1.0f,
			 0.1f, -0.1f, 0.0f,  1.0f, 0.0f
		};

		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &this->VBO);

		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		for (GLuint i = 0; i < this->amount; i++) {
			Particle particle;
			particle.position = glm::vec3(0.0f);
			particle.velosity = glm::vec3(0.0f);
			particle.acceleration = glm::vec3(0.0f);
			particle.color = glm::vec4(0.0f);
			particle.size = 0.0f;
			particle.life = 0.0f;
			this->particles.push_back(particle);
		}
	}

	GLuint lastUsedParticle = 0;
	GLuint firstUnusedParticle() {
		for (GLuint i = lastUsedParticle; i < this->amount; i++) {
			if (this->particles[i].life <= 0.0f) {
				lastUsedParticle = i;
				return i;
			}
		}
		for (GLuint i = 0; i < lastUsedParticle; i++) {
			if (this->particles[i].life <= 0.0f) {
				lastUsedParticle = i;
				return i;
			}
		}
		lastUsedParticle = 0;
		return 0;
	}

	void respawnParticle(Particle &particle) {
		// ����-1��1֮�������
		float pos_rand_x = (float)rand() / (float)(RAND_MAX / 2) - 1.0f;
		float pos_rand_y = (float)rand() / (float)(RAND_MAX / 2) - 1.0f;
		float pos_rand_z = (float)rand() / (float)(RAND_MAX / 2) - 1.0f;
		float vel_rand_x = (float)rand() / (float)(RAND_MAX / 2) - 1.0f;
		float vel_rand_y = (float)rand() / (float)(RAND_MAX / 2) - 1.0f;
		float vel_rand_z = (float)rand() / (float)(RAND_MAX / 2) - 1.0f;
		// ����0-1֮�������
		float color_rand_r = (float)rand() / (float)RAND_MAX;
		float color_rand_g = (float)rand() / (float)RAND_MAX;
		float color_rand_b = (float)rand() / (float)RAND_MAX;
		// ��������λ��
		particle.position = glm::vec3(pos_rand_x, pos_rand_y, pos_rand_z);
		// ���������ٶ�
		particle.velosity = glm::vec3(vel_rand_x, vel_rand_y, vel_rand_z);
		// �������Ӽ��ٶ�
		particle.acceleration = glm::vec3(0.0f, -0.2f, 0.0f);
		// ����������ɫ
		particle.color = glm::vec4(color_rand_r, color_rand_g, color_rand_b, 1.0f);
		// �������Ӵ�С
		particle.size = 0.05f;
		// ������������
		particle.life = 1.0f;
	}
};