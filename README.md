# SiMPSPRite

*(This is a copy of the comment at the top of simpsprite.hpp)*

SiMPSPRite is a simple (its only selling point) 2D sprite rendering library
for OpenGL 3.3+. It does not do much, but it provides basically every feature
needed to implement basic sprite functionality. It requires a functional 
OpenGL 3.3+ context with GLFW and GLAD initialized, it also requires stbi and glm.
Working versions of all the dependencies are bundled for quick deployment.
Also bundled are simple vertex and fragment shaders to be used with the library. 
For guidelines on custom shaders see the Shaders section below.


## USAGE: 
create a .cpp file containing:

  #define SIMPSPRITE_IMPLEMENTATION
  #include "simpsprite.hpp"

and nothing else

In files using the library:
`#include "simpsprite.hpp"`

## DOCUMENTATION:

### TYPES:

#### Sprite:

Sprite is a class representing one sprite (as suggested by the name). 

##### Attributes:
1. **sprite_id:** id with which the sprite can be accessed after it is stored 

2. **x,y,w,h,angle:** True X position, True Y position, width, height, angle.

3. **draw_x, draw_y, draw_a:** Displayed x Position, displayed y position, displayed h position

4. **speed:** speed at which draw_x and draw_y approach x and y, if the sprite is animated.

5. **active:** sprite is only rendered while active.

6. **animated:** the animation system functions by slowly incrementing/decrementing draw_x,
draw_y, and draw_a to approach x, y, and angle starting when any of these parameters is set to a new value.
This only takes place when animated,non animated sprites snap to the new location.

7. **layer:** The library uses a form of the painter's algorithm. Sprites with a higher layer value appear on top. 

8. **texture:** the OpenGL texture for the sprite, loaded from a .png file

9. **tint:** a vector represented as {r,g,b,a} which is multiplied to the sprite's values, {1,1,1,1} is no tint.

##### METHODS:

###### animate:
Internal, runs the animation algorithm described above.

###### tint_sprite:
takes a vector, sets the tint value

###### change_tex: 
takes a file path to a .png image. creates a texture from the image and sets the Sprite's texture field to it.


##### CORRECT WAY TO INITIALIZE A SPRITE:

`Sprite& sprite = sprite_to_screen(Sprite(1, 100, 100, 32, 32, 0, "sprite.png", true), 0);`

see sprite_to_screen below.

#### Screen: 

contains a list of sprites as well as an ID it can be referenced with.

### MAIN FUNCTIONS:

###### load_shader_program: 
takes two file paths, path to the vertex shader and path to the fragment shader.
creates a shader program and loads it.

*PLANNED: Integrating prebuilt shaders into the library.*

###### init_quad:
initializes the quadrilateral that is transformed into the various sprites.
To be called once before the render loop runs.

###### begin_frame:
set up the projection to allow pixel based coordinates. to be called in the render loop 
before show() or any other SimpSprite function is called.

###### show: 
The main rendering function, iterates over all the sprites in the active screen and renders them
based on their layer and position. Must be called in the render loop AFTER begin_frame().

###### sprite_to_screen: 
takes a sprite and an integer representing screen ID and inserts the sprite into the screen corresponding to the ID.
If this reference is lost it can be refetched using get_sprite(sprite_id)

As such, the correct way to initialize a sprite is, for example:

`Sprite& sprite = sprite_to_screen(Sprite(1, 100, 100, 32, 32, 0, "sprite.png", true), 0);`


###### get_sprite/remove_sprite: 
takes a sprite_id and a screen and retrieves/removes the desired sprite.

###### get_layer_count, set_layer_count:
get and set layer count, which is the highest possible layer.

###### get_active_screen, set_active_screen:
get and set active screen, which is the screen whose contents get rendered in the show() call.

###### add_screen, remove_screen:
allows adding and removing screens, takes the desired screen ID. Adding will decline if the screen ID is already taken.
Removal will remove the screen with the given ID and set the current screen to 0.

###### init_gl:
**FOR BEGINNERS.** Optionally takes a window width, height, and name, and initializes an OpenGL
window with those parameters, otherwise creates a 1600x900 window titled "SimpSprite Window".
Called as such:
    
    GLFWwindow* window = smpspr::init_gl(x,y,name); 
    
    And at the end of the render loop:

    glfwSwapBuffers(window);
    glfwPollEvents();

###### shutdown:
deletes all OpenGL objects, to be called directly before OpenGL context termination (if needed).

## Shaders:

The library is bundled with a set of shaders that are designed to work with
it, however, it supports the use of custom shaders, with some requirements:

- Target GLSL #version 330 core or higher.
- Uniforms are set by name, so the names below must match exactly:

1. Vertex shader:
    - Input: layout(location = 0) in vec2 aCorner; (unit quad corners, the
      only attribute)
    - Uniforms: mat4 uProj (pixel-space projection, origin top left),
      mat4 uModel (per-sprite transform).
    - Output position must be: uProj * uModel * vec4(aCorner, 0.0, 1.0)
    - Use aCorner as the UV (images are not flipped on load). Anything else
      passed to the fragment shader is up to you.

2. Fragment shader:
    - Uniforms: sampler2D uTex (sprite texture, unit 0), vec4 uTint
      (multiply into the final color).
    - Must output a single RGBA vec4. Alpha blending is on, so output alpha
      controls transparency.
