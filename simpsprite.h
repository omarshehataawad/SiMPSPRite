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
USAGE: 
create a .cpp file containing:

#define SIMPSPRITE_IMPLEMENTATION
#include "simpsprite.h"

and nothing else

In files using the library:
#include "simpsprite.h"

=======================================================================
DOCUMENTATION
=======================================================================

TYPES
-----

Sprite:
    Sprite is a struct representing one sprite (as suggested by the name).

    Attributes:
    1. sprite_id: id with which the sprite can be accessed after it is stored

    2. x, y, w, h, angle: True X position, True Y position, width, height,
       angle.

    3. draw_x, draw_y, draw_a: Displayed x Position, displayed y position,
       displayed angle

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

    
Screen:
    contains a list of sprites as well as an ID it can be referenced with.


MAIN FUNCTIONS
--------------

smpspr_load_shader_program:
    takes two file paths, path to the vertex shader and path to the fragment
    shader. creates a shader program and loads it.

    eg: smpspr_load_shader_program("vertex_shader_path", "fragment_shader_path");

smpspr_init_quad:
    initializes the quadrilateral that is transformed into the various
    sprites. To be called once before the render loop runs.

    eg: in main, after initializing viewport: smpspr_init_quad(void);

smpspr_begin_frame:
    set up the projection to allow pixel based coordinates. to be called in
    the render loop before show() or any other SimpSprite function is called.

    eg: in render loop, after clearing color: smpspr_begin_frame(width, height, window);

smpspr_show:
    The main rendering function, iterates over all the sprites in the active
    screen and renders them based on their layer and position. Must be called
    in the render loop AFTER begin_frame().

    eg: in render loop, after smpspr_begin_frame(void): smpspr_show(void);

smpspr_attach_sprite:
    takes a sprite and an integer representing screen ID and inserts the
    sprite into the screen corresponding to the ID. Returns a reference to the
    sprite in the screen. If this reference is lost it can be refetched using 
    get_sprite(sprite_id, screen_id). 

    eg: smpspr_attach_sprite(sprite,screen);

smpspr_get_sprite/smpspr_remove_sprite: 
    takes a sprite_id and a screen id and retrieves/removes the desired sprite.

    eg: smpspr_get_sprite(sprite_id, screen_id);
        smpspr_remove_sprite(sprite_id, screen_id);

smpspr_get_layer_count, smpspr_set_layer_count:
    get and set layer count for a screen, which is the highest possible layer + 1.

    eg: smpspr_get_layer_count(screen_id);
        smpspr_set_layer_count(screen_id, desired_no_of_layers);

smpspr_get_active_screen, smpspr_set_active_screen:
    get and set active screen, which is the screen whose contents get
    rendered in the show() call.

    eg: smpspr_get_active_screen();
        smpspr_set_active_screen(screen_id);

smpspr_add_screen, smpspr_remove_screen:
    allows adding and removing screens, takes the desired screen ID. Adding
    will decline if the screen ID is already taken. Removal will remove the
    screen with the given ID and set the current screen to 0.

    eg: smpspr_add_screen(screen_id);
        smpspr_remove_screen(screen_id);


smpspr_shutdown:
    deletes all OpenGL objects, to be called directly before OpenGL context
    termination (if needed).

    eg: smpspr_shutdown(void);

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
#ifndef SIMPSPRITE_H
#define SIMPSPRITE_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SIMPSPRITE_IMPLEMENTATION
    #define STB_IMAGE_IMPLEMENTATION
    #include <stb_image.h>

    #include <math.h>
#endif

//=======================================================================
// STRUCTS
//=======================================================================

typedef struct Sprite
{
    int sprite_id;
    float x,y,w,h;
    float draw_x, draw_y, draw_a;
    float speed;
    float angle;
    int active;
    int animated;
    int layer;
    unsigned int texture;
    float tint[4];
}Sprite;

typedef struct SpriteNode SpriteNode;

struct SpriteNode
{
    Sprite* sprite;
    SpriteNode* next;
};

typedef struct {
    int screen_id;
    int layer_count;
    SpriteNode* head;
    SpriteNode* tail;
} Screen;


//============================================================
// PUBLIC FUNCTIONS
//============================================================

GLuint smpspr_load_shader_program(const char* vert_path, const char* frag_path);

void smpspr_init_quad(void);

void smpspr_begin_frame(GLuint shader, float screen_w, float screen_h);

void smpspr_show(void);

Sprite* smpspr_attach_sprite(const Sprite* spr, int id);

Sprite* smpspr_get_sprite(int sprite_id, int screen_id);
void smpspr_remove_sprite(int sprite_id, int screen_id);

int smpspr_get_layer_count(int screen_id);
void smpspr_set_layer_count(int screen_id, int x);

int smpspr_get_active_screen(void);
void smpspr_set_active_screen(int X);

void smpspr_add_screen(int id);
void smpspr_remove_screen(int id);

void smpspr_shutdown(void);

void smpspr_init_sprite(
    Sprite* s,
    int id,
    float x, float y,
    float w, float h,
    int layer,
    const char* texture,
    int anim,
    float angle);

void smpspr_tint_sprite(Sprite* s, const float tint[4]);
void smpspr_change_tex(Sprite* s, const char* new_tex);

#ifdef __cplusplus
}
#endif

//============================================================
// SIMPSPRITE - IMPLEMENTATION
//============================================================

#ifdef SIMPSPRITE_IMPLEMENTATION

static Screen* screens = NULL;
static int screen_count = 0, screen_cap = 0;
static int active_screen_id=0;

static GLuint quad_vao = 0;
static GLuint quad_vbo = 0;
static GLuint current_shader = 0;

typedef struct {
    char*    path;
    GLuint   tex;
} TexEntry;

static TexEntry* texture_cache = NULL;
static int tex_count = 0, tex_cap = 0;

static char* copy_str(const char* s) {
    size_t n = strlen(s) + 1;
    char* p = (char*)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

//============================================================
// INTERNAL FUNCTIONS
//============================================================

static char* read_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        printf("Cannot open %s\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buf = (char*)malloc(size + 1);
    if (!buf) { fclose(f); return NULL; }

    size_t got = fread(buf, 1, size, f);
    buf[got] = '\0';
    fclose(f);
    return buf;
}

static GLuint compile_shader(GLenum type, const char* path)
{ 
    char* src = read_file(path);
    if (!src) return 0;

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char**)&src, NULL);
    free(src);
    glCompileShader(shader);

    GLint ok; 
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if(!ok){
        char log[1024];
        glGetShaderInfoLog(shader, 1024, NULL, log);
        printf("Shader error in %s:\n%s\n", path, log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint load_texture(const char* path){
    int w, h, ch;
    unsigned char* data = stbi_load(path, &w, &h, &ch, 4);

    if(!data) {
        printf("failed to load %s: %s\n", path, stbi_failure_reason());
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

static GLuint get_texture(const char* path){
    for (int i = 0; i < tex_count; i++)              
        if (strcmp(texture_cache[i].path, path) == 0)
            return texture_cache[i].tex;

    GLuint tex = load_texture(path);
    if (tex == 0) return 0;

    if (tex_count == tex_cap) {
        int ncap = tex_cap ? tex_cap * 2 : 8;
        TexEntry* p = (TexEntry*)realloc(
            texture_cache, ncap * sizeof(TexEntry)
        );

        if (!p) {
            glDeleteTextures(1, &tex);
            return 0;
        }

        texture_cache = p;
        tex_cap = ncap;
    }

    char* copied_path = copy_str(path);

    if (!copied_path) {
        glDeleteTextures(1, &tex);
        return 0;
    }

    texture_cache[tex_count].path = copied_path;
    texture_cache[tex_count].tex = tex;
    tex_count++;

    return tex;
}

static void make_model(float m[16], float x, float y, float w, float h, float angle_deg) {
    float a = angle_deg * 3.14159265f / 180.0f;
    float c = cosf(a), s = sinf(a);
    m[0] = c*w;   m[1] = s*w;  m[2] = 0;  m[3] = 0;
    m[4] = -s*h;  m[5] = c*h;  m[6] = 0;  m[7] = 0;
    m[8] = 0;     m[9] = 0;    m[10] = 1; m[11] = 0;
    m[12] = x + w/2 - c*w/2 + s*h/2;
    m[13] = y + h/2 - s*w/2 - c*h/2;
    m[14] = 0;    m[15] = 1;
}

static void make_ortho(float m[16],float left, float right, float bottom, float top,float near, float far)
{
    m[0]  =  2.0f / (right - left);
    m[1]  =  0.0f;
    m[2]  =  0.0f;
    m[3]  =  0.0f;

    m[4]  =  0.0f;
    m[5]  =  2.0f / (top - bottom);
    m[6]  =  0.0f;
    m[7]  =  0.0f;

    m[8]  =  0.0f;
    m[9]  =  0.0f;
    m[10] = -2.0f / (far - near);
    m[11] =  0.0f;

    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(far + near) / (far - near);
    m[15] =  1.0f;
}


static void draw_sprite(GLuint tex, float x, float y, float w, float h, float angle, const float tint[4])
{
    float mt[16];
    make_model(mt, x, y, w, h, angle);

    glUniformMatrix4fv(glGetUniformLocation(current_shader, "uModel"), 1, GL_FALSE, mt);
    glUniform4fv(glGetUniformLocation(current_shader, "uTint"), 1, tint);

    glBindTexture(GL_TEXTURE_2D, tex);

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

static void draw_helper(int layer, const SpriteNode* head)
{
    for (const SpriteNode* n = head; n; n = n->next) {
        const Sprite* s = n->sprite;
        if (s->layer == layer && s->active) {
            draw_sprite(s->texture, s->draw_x, s->draw_y, s->w, s->h, s->draw_a, s->tint);
        }
    }
}

static void animate(Sprite* s, float dt);
static void draw(const SpriteNode* sprite_head, int layer_count)
{
    static double prev = 0.0;
    static int initialized = 0;

    double cur = glfwGetTime();

    if (!initialized) {
        prev = cur;
        initialized = 1;
    }

    float dt = (float)(cur - prev);
    prev = cur;

    const SpriteNode* current = sprite_head;

    while (current != NULL)
    {
        animate(current->sprite, dt);
        current = current->next;
    }

    for (int i = 0; i < layer_count; i++)
    {
        draw_helper(i, sprite_head);
    }
}

static Screen* get_screen(int screen_id)
{
    for (int i = 0; i < screen_count; i++)
    {
        if (screens[i].screen_id == screen_id)
            return &screens[i];
    }

    return NULL;
}

static void animate(Sprite* s, float dt)
{
    if (s->animated)
    {
        float dx = s->x - s->draw_x;
        float dy = s->y - s->draw_y;
        float da = fmodf(s->angle - s->draw_a, 360.0f);

        if (da > 180.0f)
            da -= 360.0f;
        else if (da < -180.0f)
            da += 360.0f;

        float dist = sqrtf(dx * dx + dy * dy);
        float step = s->speed * dt;
        float a_step = s->speed * 5.0f * dt;

        if (s->speed * 5.0f <= 0.0f || fabsf(da) <= a_step)
            s->draw_a = s->angle;
        else
            s->draw_a += (da > 0.0f ? a_step : -a_step);

        if (s->speed <= 0.0f || dist <= step)
        {
            s->draw_x = s->x;
            s->draw_y = s->y;
            return;
        }

        s->draw_x += dx / dist * step;
        s->draw_y += dy / dist * step;
    }
    else
    {
        s->draw_x = s->x;
        s->draw_y = s->y;
        s->draw_a = s->angle;
    }
}

//============================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
//============================================================

GLuint smpspr_load_shader_program(const char* vertPath, const char* fragPath){
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
        glGetProgramInfoLog(program, 1024, NULL, log);
        printf("Link error: \n%s\n", log);
        glDeleteProgram(program);
        glDeleteShader(vert);
        glDeleteShader(frag);
        return 0;
    }
    
    glDeleteShader(vert);
    glDeleteShader(frag);
    
    return program;
}

void smpspr_init_quad()
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

void smpspr_begin_frame(GLuint shader, float screenW, float screenH)
{
    current_shader = shader;
    glUseProgram(shader);

    float proj[16];
    make_ortho(proj, 0.0f, screenW, screenH, 0.0f, -1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glUniformMatrix4fv(glGetUniformLocation(shader,"uProj"), 1, GL_FALSE, proj);
    glUniform1i(glGetUniformLocation(shader, "uTex"), 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void smpspr_show(void)
{
    for (int i = 0; i < screen_count; i++)
    {
        if (screens[i].screen_id == active_screen_id)
        {
            draw(screens[i].head, screens[i].layer_count);
            break;
        }
    }
}

Sprite* smpspr_attach_sprite(const Sprite* spr, int id)
{
    for (int i = 0; i < screen_count; i++)
    {
        Screen* s = &screens[i];

        if (s->screen_id == id)
        {
            Sprite* new_sprite = (Sprite*)malloc(sizeof(Sprite));
            if (new_sprite == NULL)
                return NULL;

            *new_sprite = *spr;

            SpriteNode* new_node = (SpriteNode*)malloc(sizeof(SpriteNode));
            if (new_node == NULL)
            {
                free(new_sprite);
                return NULL;
            }

            new_node->sprite = new_sprite;
            new_node->next = NULL;

            if (s->tail != NULL)
                s->tail->next = new_node;
            else
                s->head = new_node;

            s->tail = new_node;

            return new_sprite;
        }
    }

    return NULL;
}

void smpspr_remove_sprite(int sprite_id, int screen_id)
{
    for (int i = 0; i < screen_count; i++)
    {
        Screen* s = &screens[i];

        if (s->screen_id != screen_id)
            continue;

        SpriteNode* current = s->head;
        SpriteNode* prev = NULL;

        while (current != NULL)
        {
            if (current->sprite->sprite_id == sprite_id)
            {
                if (prev != NULL)
                    prev->next = current->next;
                else
                    s->head = current->next;

                if (s->tail == current)
                    s->tail = prev;

                free(current->sprite);
                free(current);

                return;
            }

            prev = current;
            current = current->next;
        }

        return;
    }
}

int smpspr_get_layer_count(int screen_id)
{
    Screen* s = get_screen(screen_id);
    return s ? s->layer_count : -1;
}

void smpspr_set_layer_count(int screen_id, int x)
{
    Screen* s = get_screen(screen_id);
    if (s)
        s->layer_count = x;
}

int smpspr_get_active_screen(void)
{
    return active_screen_id;
}

void smpspr_set_active_screen(int x)
{
    active_screen_id = x;
}

void smpspr_add_screen(int id)
{
    for (int i = 0; i < screen_count; i++)
    {
        if (screens[i].screen_id == id)
        {
            printf("screen id already in use\n");
            return;
        }
    }

    if (screen_count == screen_cap)
    {
        int new_cap = screen_cap == 0 ? 4 : screen_cap * 2;

        Screen* temp = (Screen*)realloc(screens, new_cap * sizeof(Screen));
        if (temp == NULL)
            return;

        screens = temp;
        screen_cap = new_cap;
    }

    Screen* s = &screens[screen_count++];

    s->screen_id = id;
    s->layer_count = 0;
    s->head = NULL;
    s->tail = NULL;
}

void smpspr_remove_screen(int id)
{
    for (int i = 0; i < screen_count; i++)
    {
        if (screens[i].screen_id != id)
            continue;

        if (id == active_screen_id)
            active_screen_id = 0;

        SpriteNode* current = screens[i].head;

        while (current != NULL)
        {
            SpriteNode* next = current->next;

            free(current->sprite);
            free(current);

            current = next;
        }

        memmove(
            &screens[i],
            &screens[i + 1],
            (screen_count - i - 1) * sizeof(Screen)
        );

        screen_count--;
        return;
    }
}

void smpspr_shutdown(void)
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

    for (int i = 0; i < screen_count; i++) {
        SpriteNode* current = screens[i].head;

        while (current != NULL) {
            SpriteNode* next = current->next;

            free(current->sprite);
            free(current);

            current = next;
        }
    }

    free(screens);
    screens = NULL;
    screen_count = 0;
    screen_cap = 0;
    active_screen_id = 0;

    for(int i = 0; i<tex_count; i++)
    {
        glDeleteTextures(1, &texture_cache[i].tex);
        free(texture_cache[i].path);
    }

    free(texture_cache);

    texture_cache = NULL;
    tex_count = 0;
    tex_cap = 0;

    current_shader = 0;
}

void smpspr_init_sprite(
    Sprite* s,
    int id,
    float x, float y,
    float w, float h,
    int layer,
    const char* texture,
    int anim,
    float angle)
{
    s->sprite_id = id;
    s->x = x;
    s->y = y;
    s->w = w;
    s->h = h;

    s->angle = angle;
    s->draw_x = x;
    s->draw_y = y;
    s->draw_a = angle;
    s->speed = 200.0f;

    s->active = 1;
    s->animated = anim;
    s->layer = layer;

    s->texture = (texture == NULL || texture[0] == '\0')? 0: get_texture(texture);

    for (int i = 0; i < 4; i++)
        s->tint[i] = 1.0f;
}

void smpspr_tint_sprite(Sprite* s, const float tint[4])
{
    for (int i = 0; i < 4; i++)
        s->tint[i] = tint[i];
}

void smpspr_change_tex(Sprite* s, const char* new_tex)
{
    s->texture = (new_tex == NULL || new_tex[0] == '\0')? 0: get_texture(new_tex);
}
#endif

#endif