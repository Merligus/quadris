#ifndef __pieces_h
#define __pieces_h

#include "shader_s.h"

class Piece
{
	friend class Grid;

public:
	enum class types { L, J, I, O, S, Z, T };
	enum class rotation { R0, R90, R180, R270 };
	types type;

	Piece(Shader s, types t, rotation r)
	{
		rot = r;
		type = t;

		// load image, create texture and generate mipmaps
		int width, height, nrChannels;
		unsigned char *data;
		
		setGeoForm(s);

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
		data = stbi_load("resources/textures/texture.jpg", &width, &height, &nrChannels, 0);
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

		s.setInt("text1", 0);
	}

	void setGeoForm(Shader s)
	{
		switch (type)
		{
		case Piece::types::L:
			positions[0] = glm::vec3(1.0f, -0.5f, 0.0f);
			positions[1] = glm::vec3(0.0f, -0.5f, 0.0f);
			positions[2] = glm::vec3(-1.0f, -0.5f, 0.0f);
			positions[3] = glm::vec3(1.0f, 0.5f, 0.0f);
			color = glm::vec3(1.0f, 0.647f, 0.0f);
			break;
		case Piece::types::J:
			positions[0] = glm::vec3(1.0f, -0.5f, 0.0f);
			positions[1] = glm::vec3(0.0f, -0.5f, 0.0f);
			positions[2] = glm::vec3(-1.0f, -0.5f, 0.0f);
			positions[3] = glm::vec3(-1.0f, 0.5f, 0.0f);
			color = glm::vec3(0.0f, 0.0f, 1.0f);
			break;
		case Piece::types::I:
			positions[0] = glm::vec3(0.0f, 1.5f, 0.0f);
			positions[1] = glm::vec3(0.0f, 0.5f, 0.0f);
			positions[2] = glm::vec3(0.0f, -0.5f, 0.0f);
			positions[3] = glm::vec3(0.0f, -1.5f, 0.0f);
			color = glm::vec3(0.0f, 1.0f, 1.0f);
			break;
		case Piece::types::O:
			positions[0] = glm::vec3(0.5f, 0.5f, 0.0f);
			positions[1] = glm::vec3(0.5f, -0.5f, 0.0f);
			positions[2] = glm::vec3(-0.5f, -0.5f, 0.0f);
			positions[3] = glm::vec3(-0.5f, 0.5f, 0.0f);
			color = glm::vec3(1.0f, 1.0f, 0.0f);
			break;
		case Piece::types::S:
			positions[0] = glm::vec3(0.0f, 0.5f, 0.0f);
			positions[1] = glm::vec3(0.0f, -0.5f, 0.0f);
			positions[2] = glm::vec3(1.0f, 0.5f, 0.0f);
			positions[3] = glm::vec3(-1.0f, -0.5f, 0.0f);
			color = glm::vec3(0.0f, 1.0f, 0.0f);
			break;
		case Piece::types::Z:
			positions[0] = glm::vec3(0.0f, 0.5f, 0.0f);
			positions[1] = glm::vec3(0.0f, -0.5f, 0.0f);
			positions[2] = glm::vec3(1.0f, -0.5f, 0.0f);
			positions[3] = glm::vec3(-1.0f, 0.5f, 0.0f);
			color = glm::vec3(1.0f, 0.0f, 0.0f);
			break;
		case Piece::types::T:
			positions[0] = glm::vec3(0.0f, 0.5f, 0.0f);
			positions[1] = glm::vec3(0.0f, -0.5f, 0.0f);
			positions[2] = glm::vec3(1.0f, -0.5f, 0.0f);
			positions[3] = glm::vec3(-1.0f, -0.5f, 0.0f);
			color = glm::vec3(0.627f, 0.125f, 0.941f);
			break;
		default:
			break;
		}
	}

	void setModel(glm::mat4 m)
	{
		model = m;
	}

	void translate(glm::vec3 t)
	{
		model = glm::translate(model, (glm::vec3)(model * glm::vec4(t, 0.0f)));
	}

	void rotate(bool d)
	{
		if (d)
			rot = (rotation)(((int)rot + 1) % 4);
		else
			rot = (rotation)(((int)rot - 1) % 4);
	}

	// angle in degrees
	void rotate(glm::vec3 r, float angle)
	{
		model = glm::rotate(model, glm::radians(angle), r);
	}

	void draw(Shader s)
	{
		glm::mat4 aux_model;

		// bind textures on corresponding texture units
		s.setBool("filled", true);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		for (unsigned int i = 0; i < 4; i++)
		{
			aux_model = glm::translate(model, positions[i]);
			s.setMat4("model", aux_model);
			s.setVec3("color", color);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}

private:
	glm::vec3 positions[4], color;
	glm::mat4 model;
	unsigned int texture;
	rotation rot;
};

#endif // !__pieces_h