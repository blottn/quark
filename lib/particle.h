class ParticleEffect {
private:
    int vao;

public:
    int id;
    std::vector<Transform *> * replications;
    glm::vec3 emitter;
    int rad;

    float verts[9] = {
        1,-1,0,
        -1,-1,0,
        0,1,0
    };

    float norms[9] = {
        0,1,0,
        0,1,0,
        0,1,0,
    };

    ParticleEffect(glm::vec3 e, GLuint shaderId, float r) {
        id = shaderId;
        emitter = e;
        rad = r;
        replications = new vector<Transform *>();
        initData();
    }
    
    void addRepl(Transform * repl) {
        replications->push_back(repl);
    }

    void initData() {
        GLuint VAOs[1];
        glGenVertexArrays(1, VAOs);
        vao = VAOs[0];
        glBindVertexArray(vao);

        unsigned int vp_vbo = 0;
        GLuint pos = glGetAttribLocation(id, "vertex_position");
        GLuint norm = glGetAttribLocation(id, "vertex_normal");

        glGenBuffers(1, &vp_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), &verts, GL_STATIC_DRAW);

        unsigned int vn_vbo = 0;
        glGenBuffers(1, &vn_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
        glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), &norms, GL_STATIC_DRAW);

        glEnableVertexAttribArray(pos);
        glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(norm);
        glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
        glVertexAttribPointer(norm, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    }

    void draw(glm::mat4 view, glm::mat4 proj) {
        glBindVertexArray(vao);
        glDepthFunc(GL_LESS);
        glUseProgram(id);

	    int matrix_location = glGetUniformLocation(id, "model");
		int view_mat_location = glGetUniformLocation(id, "view");
		int proj_mat_location = glGetUniformLocation(id, "proj");

		// update uniforms & draw
		glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, value_ptr(proj));
		glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, value_ptr(view));
        for (Transform * repl : *replications) {
		    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(repl->computeParticle()));
            glDrawArrays(GL_TRIANGLES,0,sizeof(verts));
        }
    }

    void update() {
        double randx = std::rand() * 2 - 1;
        double randy = std::rand() * 2 - 1;
        double randz = std::rand() * 2 - 1;

        glm::vec3 trans = glm::normalize(glm::vec3(randx, randy, randz));
        trans.x *= rad /2;
        trans.y *= rad /2;
        trans.z *= rad /2;
        Transform * t = new Transform();
        t->translate = glm::translate(t->translate, trans);
        t->scale = glm::scale(t->scale, glm::vec3(0.04, 0.04, 0.04));
        if (replications->size() < 100)
            this->replications->push_back(t);
        else 
            replications->erase(replications->begin());
    }
};
