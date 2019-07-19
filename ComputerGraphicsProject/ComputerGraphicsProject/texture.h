#pragma once
#include <glad/glad.h>

class Texture2D {
public:
	// 纹理ID
	unsigned int ID;
	// 纹理图像宽度和高度
	unsigned int Width, Height;
	// 纹理格式
	unsigned int Internal_Format;
	unsigned int Image_Format;
	// 纹理配置
	unsigned int Wrap_S;
	unsigned int Wrap_T;
	unsigned int Filter_Min;
	unsigned int Filter_Mag;

	// 构造函数
	Texture2D() {
		this->Width = 0;
		this->Height = 0;

		this->Internal_Format = GL_RGB;
		this->Image_Format = GL_RGB;

		this->Wrap_S = GL_REPEAT;
		this->Wrap_T = GL_REPEAT;
		this->Filter_Min = GL_LINEAR_MIPMAP_LINEAR;
		this->Filter_Mag = GL_LINEAR;

		glGenTextures(1, &this->ID);
	}

	// 根据图像数据生成纹理
	void Generate(unsigned int width, unsigned int height, unsigned char *data) {
		this->Width = width;
		this->Height = height;

		// 绑定纹理
		glBindTexture(GL_TEXTURE_2D, this->ID);

		// 生成纹理
		glTexImage2D(GL_TEXTURE_2D, 0, this->Internal_Format, width, height, 0, this->Image_Format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// 为当前绑定的纹理对象设置环绕和过滤方式
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Mag);

		// 解除绑定
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// 绑定纹理
	void Bind() const {
		glBindTexture(GL_TEXTURE_2D, this->ID);
	}
};