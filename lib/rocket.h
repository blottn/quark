class Rocket {
public:
    GLuint vao;
	GLuint shaderID;

    float verts[54] = {
        //front left
        0.0,1.0,0.0,
        -1.0,-1.0,0.0,
        0.0,0.0,0.5,

        //front right
        1.0,-1.0,0.0,
        0.0,1.0,0.0,
        0.0,0.0,0.5,

        //back left
        0.0,1.0,0.0,
        0.0,0.0,-0.5,
        -1.0,-1.0,0.0,

        //back right
        0.0,0.0,-0.5,
        0.0,1.0,0.0,
        1.0,-1.0,0.0,

        //bottom left
        0.0,0.0,-0.5,
        0.0,0.0,0.5,
        -1.0,-1.0,0.0,

        //bottom right
        0.0,0.0,-0.5,
        1.0,-1.0,0.0,
        0.0,0.0,0.5
    };

    glm::vec3 bt = glm::vec3(0.0,1.0,0.0);
    glm::vec3 bm = glm::vec3(0.0,0.0,0.5);
    glm::vec3 bl = glm::vec3(-1.0,-1.0,0.0);
    glm::vec3 br = glm::vec3(1.0,-1.0,0.0);
    
    glm::vec3 nl = glm::cross((bm-bl), (bt-bl));
    glm::vec3 nr = glm::cross((bm-br), (bt-br));

    float norms[54] = {
        //front left
        -0.1,0.1,-0.9,
        -0.1,0.1,-0.9,
        -0.1,0.1,-0.9,

        //front right
        -0.1,0.1,0.9,
        -0.1,0.1,0.9,
        -0.1,0.1,0.9,

        //back left
        -0.2,0.1,0.9,
        -0.2,0.1,0.9,
        -0.2,0.1,0.9,

        //back right
        0.2,0.1,0.9,
        0.2,0.1,0.9,
        0.2,0.1,0.9,

        //bottom left
        nl.x,nl.y,nl.z,
        nl.x,nl.y,nl.z,
        nl.x,nl.y,nl.z,

        //bottom right
        nr.x,nr.y,nr.z,
        nr.x,nr.y,nr.z,
        nr.x,nr.y,nr.z
    };

	Transform * model;

	Rocket(GLuint shader, Transform * initial) {
		shaderID = shader;
		model = new Transform();

		initData();
		model = initial;
	}

	void initData() {
		// create our vao
		GLuint VAOs[1];
		glGenVertexArrays(1, VAOs);
		vao = VAOs[0];
		glBindVertexArray(vao);

		unsigned int vp_vbo = 0;
		GLuint pos = glGetAttribLocation(shaderID, "vertex_position");
		GLuint norm = glGetAttribLocation(shaderID, "vertex_normal");

		glGenBuffers(1, &vp_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

		unsigned int vn_vbo = 0;
		glGenBuffers(1, &vn_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(norms), norms, GL_STATIC_DRAW);

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

	void draw(glm::mat4 parent, glm::mat4 view, glm::mat4 projection, glm::vec3 cameraPos) {
		bindVAO();
		glUseProgram(shaderID);
        glDepthFunc(GL_LESS);
		//Declare your uniform variables that will be used in your shader
		int matrix_location = glGetUniformLocation(shaderID, "model");
		int view_mat_location = glGetUniformLocation(shaderID, "view");
		int proj_mat_location = glGetUniformLocation(shaderID, "proj");
        int camera_location = glGetUniformLocation(shaderID, "cameraPos");
		// update uniforms & draw
		glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, glm::value_ptr((parent * model->compute())));
        glUniform3f(camera_location, cameraPos.x, cameraPos.y, cameraPos.z);

		glDrawArrays(GL_TRIANGLES, 0, sizeof(verts) / sizeof(float));
	}

    void update() {

    }
};

