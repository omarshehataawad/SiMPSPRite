#pragma once

/*=======================================================================
-------------------------------SiMPSPRite--------------------------------

SiMPSPRite is a simple (its only selling point) 2D sprite rendering library
for OpenGL 3.3+. It does not do much, but it provides basically every feature
needed to implement basic sprite functionality. It requires a functional
OpenGL 3.3+ context with GLFW and GLAD initialized, it also requires stbi and
glm. Working versions of all the dependencies are bundled for quick
deployment. Also bundled are simple vertex and fragment shaders to be used
with the library. For guidelines on custom shaders see the Shaders section
below.

=======================================================================
DOCUMENTATION
=======================================================================

TYPES
-----

Sprite:
    Sprite is a class representing one sprite (as suggested by the name).

    Attributes:
    1. sprite_id: id with which the sprite can be accessed after it is stored

    2. x, y, w, h, angle: True X position, True Y position, width, height,
       angle.

    3. draw_x, draw_y, draw_a: Displayed x Position, displayed y position,
       displayed h position

    4. speed: speed at which draw_x and draw_y approach x and y, if the
       sprite is animated.

    5. active: sprite is only rendered while active.

    6. animated: the animation system functions by slowly
       incrementing/decrementing draw_x, draw_y, and draw_a to approach x, y,
       and angle starting when any of these parameters is set to a new value.
       This only takes place when animated, non animated sprites snap to the
       new location.

    7. layer: The library uses a form of the painter's algorithm. Sprites
       with a higher layer value appear on top.

    8. texture: the OpenGL texture for the sprite, loaded from a .png file

    9. tint: a vector represented as {r,g,b,a} which is multiplied to the
       sprite's values, {1,1,1,1} is no tint.

    Methods:

    animate:      Internal, runs the animation algorithm described above.

    tint_sprite:  takes a vector, sets the tint value

    change_tex:   takes a file path to a .png image. creates a texture from
                  the image and sets the Sprite's texture field to it.

Screen:
    contains a list of sprites as well as an ID it can be referenced with.


MAIN FUNCTIONS
--------------

load_shader_program:
    takes two file paths, path to the vertex shader and path to the fragment
    shader. creates a shader program and loads it.

init_quad:
    initializes the quadrilateral that is transformed into the various
    sprites. To be called once before the render loop runs.

begin_frame:
    set up the projection to allow pixel based coordinates. to be called in
    the render loop before show() or any other SimpSprite function is called.

show:
    The main rendering function, iterates over all the sprites in the active
    screen and renders them based on their layer and position. Must be called
    in the render loop AFTER begin_frame().

sprite_to_screen:
    takes a sprite and an integer representing screen ID and inserts the
    sprite into the screen corresponding to the ID. This is a COPY. 
    the sprite must be modified using get_sprite(id).attribute, the 
    original sprite is separate from the one actually in the list.

    -MAYBE PLANNED: switch from this model to shared pointers.

get_sprite/remove_sprite: 
    takes a sprite_id and a screen and retrieves/removes the desired sprite.

get_layer_count, set_layer_count:
    get and set layer count, which is the highest possible layer + 1.

get_active_screen, set_active_screen:
    get and set active screen, which is the screen whose contents get
    rendered in the show() call.

add_screen, remove_screen:
    allows adding and removing screens, takes the desired screen ID. Adding
    will decline if the screen ID is already taken. Removal will remove the
    screen with the given ID and set the current screen to 0.

init_gl:
    FOR BEGINNERS. Optionally takes a window width, height, and name, and
    initializes an OpenGL window with those parameters, otherwise creates a
    1600x900 window titled "SimpSprite Window". Called as such:
    GLFWwindow* window = smpspr::init_gl(x,y,name); 
    And at the end of the render loop:

    glfwSwapBuffers(window);
    glfwPollEvents();

shutdown():
    deletes all OpenGL objects, to be called directly before OpenGL context
    termination (if needed).


=======================================================================
SHADERS
=======================================================================

The library is bundled with a set of shaders that are designed to work with
it, however, it supports the use of custom shaders, with some requirements:

- Target GLSL #version 330 core or higher.
- Uniforms are set by name, so the names below must match exactly:

Vertex shader:
    - Input: layout(location = 0) in vec2 aCorner; (unit quad corners, the
      only attribute)
    - Uniforms: mat4 uProj (pixel-space projection, origin top left),
      mat4 uModel (per-sprite transform).
    - Output position must be: uProj * uModel * vec4(aCorner, 0.0, 1.0)
    - Use aCorner as the UV (images are not flipped on load). Anything else
      passed to the fragment shader is up to you.

Fragment shader:
    - Uniforms: sampler2D uTex (sprite texture, unit 0), vec4 uTint
      (multiply into the final color).
    - Must output a single RGBA vec4. Alpha blending is on, so output alpha
      controls transparency.
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
    #include <stdexcept>
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
        int sprite_id;
        float x,y,w,h;
        float draw_x, draw_y, draw_a;
        float speed;
        float angle;
        bool active;
        bool animated;
        int layer;
        unsigned int texture;
        glm::vec4 tint = {1,1,1,1};

    Sprite(int id = 0, float x = 0, float y = 0, float w = 0, float h = 0, int layer = 0,
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

Sprite& get_sprite(int screen_id, int sprite_id);
void remove_sprite(int screen_id, int sprite_id);

int get_layer_count();
void set_layer_count(int x);

int get_active_screen();
void set_active_screen(int X);

void add_screen(int id);
void remove_screen(int id);

GLFWwindow* init_gl(int x = 1600, int y = 900, std::string window_name = "SimpSprite Window");

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

Sprite& get_sprite(int screen_id, int sprite_id)
{
    for (Screen& s : screens) {
        if (s.screen_id != screen_id) continue;
        for (Sprite& spr : s.sprites)
            if (spr.sprite_id == sprite_id) return spr;
        break;
    }
    throw std::out_of_range("smpspr::get_sprite: no such sprite");
}

void remove_sprite(int screen_id, int sprite_id)
{
    for (Screen& s : screens) {
        if (s.screen_id != screen_id) continue;
        for (auto i = s.sprites.begin(); i != s.sprites.end(); ++i) {
            if (i->sprite_id == sprite_id) {
                s.sprites.erase(i);
                return;
            }
        }
        return;
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

GLFWwindow* init_gl(int x, int y, std::string window_name)
{
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(x, y, window_name.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return nullptr;
    }

    glViewport(0, 0, x, y);
    return window;
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

Sprite::Sprite(int id, float x, float y, float w, float h, int layer,
     const std::string& texture, bool anim, float angle)
    : sprite_id(id), x(x), y(y), w(w), h(h), angle(angle), draw_x(x), draw_y(y), draw_a(angle), speed(200),
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
