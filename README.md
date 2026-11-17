# SIMPSPRITE

(This is a copy of the comment at the top of SimpSprite.hpp)

SimpSprite is a simple (its only selling point) 2D sprite rendering library
for OpenGL 3.3+ It does not do much, but it provides basically every feature
needed to implement basic sprite functionality. It requires a functional 
OpenGL 3.3+ context with GLFW and GLAD initialized,it also requires stbi and glm.
Working versions of all the dependancies are bundled for quick deployment.

## DOCUMENTATION:

### TYPES:

#### Sprite:

Sprite is a class representing one sprite (as suggested by the name). 

##### Attributes:

1. x,y,w,h,angle: True X position, True Y position, width, height, angle.

2. draw_x, draw_y, draw_h: Displayed x Position, displayed y position, displayed h position

3. speed: speed at which draw_x and draw_y approach x and y, if the sprite is animated.

4. active: sprite is only rendered while active.

5. animated: the animation system functions by slowly incrementing/decrementing draw_x,
draw_y, and draw_a to approach x, y, and angle starting when of these parameters is set to a new value.
This only takes place when animated,non animated sprites snap to the new location.

6. layer: The library uses a form of the painter's algorithm. Sprites with a higher layer value appear on top. 

7. texture: the OpenGL texture for the sprite, loaded from a .png file

8. tint: a vector represented as {r,g,b,a} which is multiplied to the sprite's values, {1,1,1,1} is no tint.

###### METHODS:

###### animate:
Internal, runs the animation algorithm described above.

###### tint_sprite:
takes a vector, sets the tint value

###### change_tex: 
takes a file path to a .png image. creates a texture from the image and sets the Sprite's texture field to it.

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

###### get_layer_count, set_layer_count:
get and set layer count, which is the highest possible layer.

###### get_active_screen, set_active_screen:
get and set active screen, which is the screen whose contents get rendered in the show() call.

###### add_screen, remove_screen:
allows adding and removing screen, takes the desired screen ID. Adding will decline if the screen ID is already taken.
Removal will remove the screen with the given ID and set the current screen to 0.

###### init_gl:
**FOR BEGINNERS.** Optionally takes a window width, height, and name, and initializes an OpenGL
window with those parameters, otherwise creates a 1600x900 window titled "SimpSprite Window".

###### shutdown():
deletes all OpenGL objects, to be called directly before OpenGL context termination (if needed).
