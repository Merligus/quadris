#ifndef __grid_h
#define __grid_h

#include "shader_s.h"
#include "pieces.h"

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

	void fillBlock(glm::vec3 c)
	{
		filled = true;
		setColor(c);
	}

	void unfillBlock()
	{
		filled = false;
	}

	void draw(glm::mat4 model, Shader s, unsigned int text1, unsigned int text2)
	{
		// bind textures on corresponding texture units
		s.setBool("filled", filled);
		if (filled)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, text1);
		}
		else
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, text2);
		}
		glm::mat4 aux_model = glm::translate(model, glm::vec3(0.5f + column, 0.5f + line, 0.0f));
		s.setMat4("model", aux_model);
		s.setVec3("color", color);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

private:
	glm::vec3 color;
	unsigned int line, column;
};

class Grid
{
	struct coord
	{
		unsigned int x, y;

		void assign(int a, int b)
		{
			x = a;
			y = b;
		}
	};

	struct set
	{
		coord positions[4];
		
		void assign(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
		{
			positions[0].assign(x1, y1);
			positions[1].assign(x2, y2);
			positions[2].assign(x3, y3);
			positions[3].assign(x4, y4);
		}
	};

public:
	
	Grid(Shader s)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-5.0f, -10.0f, 0.0f));
		for (int l = 0; l < 20; l++)
			for (int c = 0; c < 10; c++)
				b[l][c].setPositions(l, c);

		setTexture(s);

		// start positions translated
		startPositions[(int)Piece::types::L][(int)Piece::rotation::R0].assign(17, 4, 17, 5, 18, 4, 19, 4);
		startPositions[(int)Piece::types::L][(int)Piece::rotation::R90].assign(18, 3, 18, 4, 18, 5, 19, 4);
		startPositions[(int)Piece::types::L][(int)Piece::rotation::R180].assign(17, 5, 18, 5, 19, 4, 19, 5);
		startPositions[(int)Piece::types::L][(int)Piece::rotation::R270].assign(18, 3, 19, 3, 19, 4, 19, 5);
		startPositions[(int)Piece::types::J][(int)Piece::rotation::R0].assign(17, 4, 17, 5, 18, 5, 19, 5);
		startPositions[(int)Piece::types::J][(int)Piece::rotation::R90].assign(18, 5, 19, 3, 19, 4, 19, 5);
		startPositions[(int)Piece::types::J][(int)Piece::rotation::R180].assign(17, 4, 18, 4, 19, 4, 19, 5);
		startPositions[(int)Piece::types::J][(int)Piece::rotation::R270].assign(18, 3, 18, 4, 18, 5, 19, 3);
		startPositions[(int)Piece::types::I][(int)Piece::rotation::R0].assign(16, 4, 17, 4, 18, 4, 19, 4);
		startPositions[(int)Piece::types::I][(int)Piece::rotation::R90].assign(19, 3, 19, 4, 19, 5, 19, 6);
		startPositions[(int)Piece::types::I][(int)Piece::rotation::R180].assign(16, 4, 17, 4, 18, 4, 19, 4);
		startPositions[(int)Piece::types::I][(int)Piece::rotation::R270].assign(19, 3, 19, 4, 19, 5, 19, 6);
		startPositions[(int)Piece::types::O][(int)Piece::rotation::R0].assign(18, 4, 18, 5, 19, 4, 19, 5);
		startPositions[(int)Piece::types::O][(int)Piece::rotation::R90].assign(18, 4, 18, 5, 19, 4, 19, 5);
		startPositions[(int)Piece::types::O][(int)Piece::rotation::R180].assign(18, 4, 18, 5, 19, 4, 19, 5);
		startPositions[(int)Piece::types::O][(int)Piece::rotation::R270].assign(18, 4, 18, 5, 19, 4, 19, 5);
		startPositions[(int)Piece::types::S][(int)Piece::rotation::R0].assign(18, 3, 18, 4, 19, 4, 19, 5);
		startPositions[(int)Piece::types::S][(int)Piece::rotation::R90].assign(17, 5, 18, 4, 18, 5, 19, 4);
		startPositions[(int)Piece::types::S][(int)Piece::rotation::R180].assign(18, 3, 18, 4, 19, 4, 19, 5);
		startPositions[(int)Piece::types::S][(int)Piece::rotation::R270].assign(17, 5, 18, 4, 18, 5, 19, 4);
		startPositions[(int)Piece::types::Z][(int)Piece::rotation::R0].assign(18, 4, 18, 5, 19, 3, 19, 4);
		startPositions[(int)Piece::types::Z][(int)Piece::rotation::R90].assign(17, 4, 18, 4, 18, 5, 19, 5);
		startPositions[(int)Piece::types::Z][(int)Piece::rotation::R180].assign(18, 4, 18, 5, 19, 3, 19, 4);
		startPositions[(int)Piece::types::Z][(int)Piece::rotation::R270].assign(17, 4, 18, 4, 18, 5, 19, 5);
		startPositions[(int)Piece::types::T][(int)Piece::rotation::R0].assign(18, 4, 19, 3, 19, 4, 19, 5);
		startPositions[(int)Piece::types::T][(int)Piece::rotation::R90].assign(17, 4, 18, 4, 18, 5, 19, 4);
		startPositions[(int)Piece::types::T][(int)Piece::rotation::R180].assign(18, 3, 18, 4, 18, 5, 19, 4);
		startPositions[(int)Piece::types::T][(int)Piece::rotation::R270].assign(17, 5, 18, 4, 18, 5, 19, 5);
	}

	void setTexture(Shader s)
	{
		// load image, create texture and generate mipmaps
		int width, height, nrChannels;
		unsigned char *data;

		// load and create a texture 
		// -------------------------
		glGenTextures(1, &text1);
		glBindTexture(GL_TEXTURE_2D, text1);
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

		// load and create a texture 
		// -------------------------
		glGenTextures(1, &text2);
		glBindTexture(GL_TEXTURE_2D, text2);
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

	void draw(Shader s)
	{
		for (int l = 0; l < 20; l++)
			for (int c = 0; c < 10; c++)
				b[l][c].draw(model, s, text1, text2);
		for (int l = 20; l < 24; l++)
			for (int c = 0; c < 10; c++)
				if(b[l][c].filled)
					b[l][c].draw(model, s, text1, text2);
	}

	void start(Piece p)
	{
		bool success = false;
		int offset = 0;
		coord p0, p1, p2, p3;
		
		p0 = startPositions[(int)p.type][(int)p.rot].positions[0];
		p1 = startPositions[(int)p.type][(int)p.rot].positions[1];
		p2 = startPositions[(int)p.type][(int)p.rot].positions[2];
		p3 = startPositions[(int)p.type][(int)p.rot].positions[3];

		while (!success && offset <= 4)
		{
			if (!b[p0.x + offset][p0.y].filled && !b[p1.x + offset][p1.y].filled && !b[p2.x + offset][p2.y].filled && !b[p3.x + offset][p3.y].filled)
			{
				b[p0.x + offset][p0.y].fillBlock(p.color);
				b[p1.x + offset][p1.y].fillBlock(p.color);
				b[p2.x + offset][p2.y].fillBlock(p.color);
				b[p3.x + offset][p3.y].fillBlock(p.color);
				success = true;
			}
			else
				offset++;
		}
	}

private:
	Block b[24][10];
	glm::mat4 model;
	set startPositions[7][4];
	unsigned int text1, text2;
};

#endif // !__grid_h