#pragma once

/*=======================================================================
-------------------------------SIMPSPRITE--------------------------------

SimpSprite is a simple (its only selling point) 2D sprite rendering library
for OpenGL 3.3+ It does not do much, but it provides basically every feature
needed to implement basic sprite functionality. It requires a functional 
OpenGL 3.3+ context with GLFW and GLAD initialized.

=======================================================================*/

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#ifdef SIMPSPRITE_IMPLEMENTATION
    #define STB_IMAGE_IMPLEMENTATION
    #include <stb_image.h>

    #include <fstream>
    #include <sstream>
    #include <iostream>
    #include <unordered_map>
    #include <cmath>
    #include <glm/gtc/matrix_transform.hpp>
    #include <glm/gtc/type_ptr.hpp>
#endif

namespace smpspr{
//=======================================================================
// SPRITE
//=======================================================================

class Sprite
{
    public:
        float x,y,w,h;
        float draw_x, draw_y, draw_a;
        float speed;
        float angle;
        bool active;
        bool animated;
        int layer;
        unsigned int texture;
        glm::vec4 tint = {1,1,1,1};

    Sprite(float x = 0, float y = 0, float w = 0, float h = 0, int layer = 0,
        const std::string& texture = "", bool anim = false, float angle = 0.0f);

    void animate(float dt);
    void tint_sprite(glm::vec4 tints);
    void change_tex(const std::string& new_tex);
};

//============================================================
// SCREEN
//============================================================

struct Screen {
    int screen_id;
    std::vector<Sprite> sprites;
};

//============================================================
// PUBLIC FUNCTIONS
//============================================================

GLuint load_shader_program(const std::string& vert_path, const std::string& frag_path);

void init_quad();

void begin_frame(GLuint shader, float screen_w, float screen_h);

void show();

void sprite_to_screen(Sprite& spr, int id);

int get_layer_count();
void set_layer_count(int x);

int get_active_screen();
void set_active_screen(int X);

void add_screen(int id);
void remove_screen(int id);

void init_gl(int x, int y, std::string window_name);

void shutdown();

//============================================================
// SIMPSPRITE - IMPLEMENTATION
//============================================================

#ifdef SIMPSPRITE_IMPLEMENTATION

//============================================================
// INTERNAL STATE
//============================================================
static std::vector<Screen> screens;
static int active_screen_id=0;
static int layer_count=0;

static GLuint quad_vao = 0;
static GLuint quad_vbo = 0;
static GLuint current_shader = 0;

static std::unordered_map<std::string, GLuint> texture_cache;

//============================================================
// INTERNAL FUNCTIONS
//============================================================

static GLuint compile_shader(GLenum type, const std::string& path)
{ 
    std::ifstream foil(path);
    if(!foil)
    {
        std::cout << "Cannot open " << path << "\n";
        return 0;
    } 
    std::stringstream ss;
    ss << foil.rdbuf();
    std::string src = ss.str();
    const char* c_src = src.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &c_src, nullptr);
    glCompileShader(shader);

    GLint ok; 
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if(!ok){
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        std::cout << "Shader error in " << path << ":\n" << log << "\n";
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint load_texture(const std::string& path){
    int w, h, ch;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 4);

    if(!data) {
        std::cout << "failed to load " << path << ":" <<stbi_failure_reason()<< "\n";
        return 0;
    }

    GLuint tex;
    glGenTextures(1, &tex);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);
    return tex;
}

static GLuint get_texture(const std::string& path){
    auto it = texture_cache.find(path);
    if(it != texture_cache.end()) return it->second;
    GLuint tex = load_texture(path);

    if (tex != 0) texture_cache[path] = tex;
    return tex;
}

static void draw_sprite(GLuint tex, float x, float y, float w, float h, float angle, glm::vec4 tint)
{
    glm::mat4 mt(1.f);

    mt = glm::translate(mt, glm::vec3(x, y, 0));

    mt = glm::translate(mt, glm::vec3(w*0.5f, h*0.5f, 0));

    mt = glm::rotate(mt, glm::radians(angle), glm::vec3(0,0,1));

    mt = glm::translate(mt, glm::vec3(-w*0.5f, -h*0.5f,0));

    mt = glm::scale(mt, glm::vec3(w,h,1));

    glUniformMatrix4fv(glGetUniformLocation(current_shader, "uModel"), 1, GL_FALSE, glm::value_ptr(mt));
    glUniform4fv(glGetUniformLocation(current_shader, "uTint"), 1, glm::value_ptr(tint));

    glBindTexture(GL_TEXTURE_2D, tex);

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

static void draw_helper(int layer, std::vector<Sprite>& sprites)
{
    for(const Sprite& sprite : sprites){
        if(sprite.layer == layer && sprite.active){
            GLuint tex = sprite.texture;
            draw_sprite(tex, sprite.draw_x, sprite.draw_y, sprite.w, sprite.h, sprite.draw_a, sprite.tint);
        }
    }
}

static void draw(std::vector<Sprite>& sprites)
{
    static double prev = glfwGetTime();
    double cur = glfwGetTime();
    float dt = (float)(cur - prev);
    prev = cur;

    for(Sprite& s : sprites){
        s.animate(dt);
    }

    for(int i = 0; i<layer_count; i++){
        draw_helper(i,sprites);
    }
}

//============================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
//============================================================

GLuint load_shader_program(const std::string& vertPath, const std::string& fragPath){
    GLuint vert = compile_shader(GL_VERTEX_SHADER, vertPath);
    GLuint frag = compile_shader(GL_FRAGMENT_SHADER, fragPath);

    if (!vert || !frag) {
        if (vert) glDeleteShader(vert);
        if (frag) glDeleteShader(frag);
        return 0;
    }
    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint ok;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if(!ok){
        char log[1024];
        glGetProgramInfoLog(program, 1024, nullptr, log);
        std::cout << "Link error: \n" << log << "\n"; 
        glDeleteProgram(program);
        glDeleteShader(vert);
        glDeleteShader(frag);
        return 0;
    }
    
    glDeleteShader(vert);
    glDeleteShader(frag);
    
    return program;
}

void init_quad()
{
    if (quad_vao != 0) return;
    float corners[] = {0,0, 1,0, 1,1, 0,1};

    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);

    glBindVertexArray(quad_vao);

    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(corners), corners, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void begin_frame(GLuint shader, float screenW, float screenH)
{
    current_shader = shader;
    glUseProgram(shader);

    glm::mat4 proj = glm::ortho(0.f, screenW, screenH, 0.f, -1.f, 1.f);

    glActiveTexture(GL_TEXTURE0);
    glUniformMatrix4fv(glGetUniformLocation(shader,"uProj"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1i(glGetUniformLocation(shader, "uTex"), 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void show()
{
    for(Screen &s : screens){
        if(s.screen_id == active_screen_id){
            draw(s.sprites);
        }
    }
}

void sprite_to_screen(Sprite& spr, int id)
{
    for (Screen& s : screens)
    {
        if (s.screen_id == id)
        {
            s.sprites.emplace_back(spr);
            return;
        }
    }
}

int get_layer_count()
{
    return layer_count;
}

void set_layer_count(int x)
{
    layer_count = x;
}

int get_active_screen()
{
    return active_screen_id;
}

void set_active_screen(int x)
{
    active_screen_id = x;
}

void add_screen(int id)
{
    for (const Screen& s : screens)
        if (s.screen_id == id){
            std::cout << "screen id already in use";
            return;
        }


    screens.emplace_back(Screen{id, {}});
}

void remove_screen(int id)
{
    if(id == active_screen_id) active_screen_id = 0;
    for (auto i = screens.begin(); i != screens.end(); ++i)
    {
        if (i->screen_id == id)
        {
            screens.erase(i);
            return;
        }
    }
}

void init_gl(int x = 1600, int y = 900, std::string window_name = "SimpSprite Window")
{
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(x, y, window_name.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return;
    }

    glViewport(0, 0, x, y);
}

void shutdown()
{
    if (quad_vbo != 0)
    {
        glDeleteBuffers(1, &quad_vbo);
        quad_vbo = 0;
    }

    if (quad_vao != 0)
    {
        glDeleteVertexArrays(1, &quad_vao);
        quad_vao = 0;
    }

    for (const auto& entry : texture_cache)
    glDeleteTextures(1, &entry.second);

    texture_cache.clear();
}

//============================================================
// CLASS IMPLEMENTATIONS
//============================================================

Sprite::Sprite(float x, float y, float w, float h, int layer,
     const std::string& texture, bool anim, float angle)
    : x(x), y(y), w(w), h(h), angle(angle), draw_x(x), draw_y(y), draw_a(angle), speed(200),
    active(true), animated(anim), layer(layer), texture(texture.empty() ? 0 : get_texture(texture)) {}

void Sprite::animate(float dt){
    if(animated){
        float dx = x - draw_x, dy = y - draw_y, da = std::fmod(angle - draw_a, 360.f);
        if(da > 180.f) da -= 360.f; 
        else if(da < -180.f) da += 360.f;

        float dist = std::sqrt(dx*dx + dy*dy);
        float step = speed * dt;
        float aStep = speed * 5 * dt;

        if(speed * 5 <= 0.f || std::fabs(da) <= aStep) draw_a = angle;
        else draw_a += (da > 0.f ? aStep : -aStep);

        if(speed <= 0.f || dist <= step){
            draw_x = x; draw_y = y;  
            return;
        }
        draw_x += dx / dist * step;
        draw_y += dy / dist * step;
    }else{
        draw_x = x;
        draw_y = y;
        draw_a = angle;
    }
}

void Sprite::tint_sprite(glm::vec4 tints){
    tint = tints;
}

void Sprite::change_tex(const std::string& new_tex){
    texture = get_texture(new_tex);
}
#endif //SIMPSPRITE_IMPLEMENTATION
}