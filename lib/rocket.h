class Rocket {
public:
    GLuint vao;

	GLuint shaderID;

	ModelData mesh_raw;
	Transform * model;

	Rocket(ModelData mesh, GLuint shader, Transform * initial) {
		shaderID = shader;
		model = new Transform();

		mesh_raw = mesh;
		initData();
		model = initial;
	}

	void initData() {
		// create our vao
		GLuint VAOs[1];
		glGenVertexArrays(1, VAOs);
		vao = VAOs[0];
		glBindVertexArray(vao);

		// black magic *waves hands*
		unsigned int vp_vbo = 0;
		GLuint pos = glGetAttribLocation(shaderID, "vertex_position");
		GLuint norm = glGetAttribLocation(shaderID, "vertex_normal");
		GLuint tex = glGetAttribLocation(shaderID, "vertex_texture");

		glGenBuffers(1, &vp_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
		glBufferData(GL_ARRAY_BUFFER, mesh_raw.mPointCount * sizeof(glm::vec3), &mesh_raw.mVertices[0], GL_STATIC_DRAW);
		unsigned int vn_vbo = 0;
		glGenBuffers(1, &vn_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
		glBufferData(GL_ARRAY_BUFFER, mesh_raw.mPointCount * sizeof(glm::vec3), &mesh_raw.mNormals[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(pos);
		glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
		glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(norm);
		glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
		glVertexAttribPointer(norm, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	}

	void bindVAO() {
		glBindVertexArray(vao);
	}

	void draw(glm::mat4 parent, glm::mat4 view, glm::mat4 projection) {
		bindVAO();
		glUseProgram(shaderID);
        glDepthFunc(GL_GREATER);
		//Declare your uniform variables that will be used in your shader
		int matrix_location = glGetUniformLocation(shaderID, "model");
		int view_mat_location = glGetUniformLocation(shaderID, "view");
		int proj_mat_location = glGetUniformLocation(shaderID, "proj");


		// update uniforms & draw
		glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));

		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, glm::value_ptr((parent * model->compute())));
		glDrawArrays(GL_TRIANGLES, 0, mesh_raw.mPointCount);
	}
};

