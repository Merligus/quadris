#ifndef __grid_h
#define __grid_h

#include "shader_s.h"

class Block
{
public:
	bool filled;

	Block()
	{
		filled = false;
		color = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	void setPositions(unsigned int l, unsigned int c)
	{
		column = c;
		line = l;
	}

	void setColor(glm::vec3 c)
	{
		color = c;
	}

	void setTexture(Shader s)
	{
		// load image, create texture and generate mipmaps
		int width, height, nrChannels;
		unsigned char *data;
		
		// load and create a texture 
		// -------------------------
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_set_flip_vertically_on_load(false); // tell stb_image.h to flip loaded texture's on the y-axis.
		data = stbi_load("resources/textures/transparent.jpg", &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);

		s.setInt("text2", 1);
	}

	void fillBlock(glm::vec3 c)
	{
		filled = true;
		setColor(c);
	}

	void unfillBlock()
	{
		filled = false;
	}

	void draw(glm::mat4 model, Shader s)
	{
		// bind textures on corresponding texture units
		s.setBool("filled", filled);
		if(filled)
			glActiveTexture(GL_TEXTURE0);
		else
			glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture);

		glm::mat4 aux_model = glm::translate(model, glm::vec3(0.5f + column, 0.5f + line, 0.0f));
		s.setMat4("model", aux_model);
		s.setVec3("color", color);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

private:
	glm::vec3 color;
	unsigned int texture, line, column;
};

class Grid
{
public:
	
	Grid(Shader s)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-5.0f, -10.0f, 0.0f));
		for (int l = 0; l < 20; l++)
		{
			for (int c = 0; c < 10; c++)
			{
				b[l][c].setPositions(l, c);
				b[l][c].setTexture(s);
			}
		}
	}

	void draw(Shader s)
	{
		for (int l = 0; l < 20; l++)
			for (int c = 0; c < 10; c++)
				b[l][c].draw(model, s);
	}

private:
	Block b[20][10];
	glm::mat4 model;
};

#endif // !__grid_h