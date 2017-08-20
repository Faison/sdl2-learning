/**
 * Phase 01 - Get a window open with SDL2.
 *
 * This could benefit from lots of cleanup and refactoring into separate functions.
 * But I'm learning, so that can wait.
 */
#include <stdlib.h>
#include <stdio.h>
#define GL4_PROTOTYPES 1
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>

#define RATE_LIMIT 20

// Define a square's points (the first four points) and a triangle's points (the latter 3).
GLfloat vertices[] = {
  // Position   Color             Texcoords
  -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
   0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
   0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
  -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
};

GLuint elements[] = {
  0, 1, 2,
  2, 3, 0
};

const char *vertex_shader =
  "#version 450 core\n"
  "in vec2 position;"
  "in vec3 color;"
  "in vec2 texcoord;"
  "out vec3 Color;"
  "out vec2 Texcoord;"
  "void main() {"
  "  Color = color;"
  "  Texcoord = texcoord;"
  "  gl_Position = vec4(position, 0.0, 1.0);"
  "}";

const char *fragment_shader =
  "#version 450 core\n"
  "in vec3 Color;"
  "in vec2 Texcoord;"
  "out vec4 outColor;"
  "uniform sampler2D tex;"
  "uniform float factor;"
  "void main() {"
  "  if (Texcoord.y < factor) {"
  "    outColor = texture(tex, Texcoord) * vec4(Color, 1.0);"
  "  } else {"
  "    outColor = texture(tex, vec2(Texcoord.x, 1.0 - Texcoord.y));"
  "  }"
  "}";

const char *GetGLErrorStr(GLenum err)
{
    switch (err)
    {
    case GL_NO_ERROR:          return "No error";
    case GL_INVALID_ENUM:      return "Invalid enum";
    case GL_INVALID_VALUE:     return "Invalid value";
    case GL_INVALID_OPERATION: return "Invalid operation";
    case GL_STACK_OVERFLOW:    return "Stack overflow";
    case GL_STACK_UNDERFLOW:   return "Stack underflow";
    case GL_OUT_OF_MEMORY:     return "Out of memory";
    default:                   return "Unknown error";
    }
}

void CheckGLError()
{
    while (1)
    {
        const GLenum err = glGetError();
        if (GL_NO_ERROR == err)
            break;

        printf("GL Error: %s\n", GetGLErrorStr(err));
    }
}

int main(int argc, char *argv[])
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

  SDL_Surface *surface = IMG_Load("codemento.jpg");
  SDL_Surface *agh = IMG_Load("aghgghghghgh.png");

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  SDL_Window *win = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  if (win == NULL) {
    printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_GLContext gl_context = SDL_GL_CreateContext(win);
  if (gl_context == NULL) {
    printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
    SDL_DestroyWindow(win);
    return EXIT_FAILURE;
  }

  SDL_GL_SetSwapInterval(1);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // Setup the shaders!
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  GLuint ebo = 0;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  glCompileShader(vs);

  GLint success = GL_FALSE;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
  if (success != GL_TRUE) {
    printf("vertex shader failed to compile!\n");
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  glCompileShader(fs);

  success = GL_FALSE;
  glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
  if (success != GL_TRUE) {
    printf("fragment shader failed to compile!\n");
  }

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vs);
  glAttachShader(shader_program, fs);
  glBindFragDataLocation(shader_program, 0, "outColor");
  glLinkProgram(shader_program);
  glUseProgram(shader_program);

  GLint posAttrib = glGetAttribLocation(shader_program, "position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

  GLint colAttrib = glGetAttribLocation(shader_program, "color");
  glEnableVertexAttribArray(colAttrib);
  glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));

  GLint texAttrib = glGetAttribLocation(shader_program, "texcoord");
  glEnableVertexAttribArray(texAttrib);
  glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void *)(5 * sizeof(GLfloat)));

  GLuint tex[] = {
    0, 0
  };

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
  glUniform1i(glGetUniformLocation(shader_program, "tex"), 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  GLfloat factorToSet = 1.0f;
  GLfloat setFactor = 0.0f;
  GLint uniFactor = glGetUniformLocation(shader_program, "factor");

  while (factorToSet > -0.1f) {
    glUniform1f(uniFactor, factorToSet);
    glGetUniformfv(shader_program, uniFactor, &setFactor);
    printf("Factor of %.1f becomes %f\n", factorToSet, setFactor);
    factorToSet -= 0.1;
  }

  glUniform1f(uniFactor, 0.6f);

  // The game loop setup.
  int running = 1;
  unsigned int last_time = 0;
  unsigned int current_time = 0;
  unsigned int mill_store = 0;

  SDL_Event event;

  last_time = SDL_GetTicks();

  // The game loop.
  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = 0;
      } else if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          running = 0;
        }
      }
    }

    current_time = SDL_GetTicks();
    mill_store += current_time - last_time;

    if ( mill_store > RATE_LIMIT && running) {
      for (; mill_store > RATE_LIMIT && running; mill_store -= RATE_LIMIT) {

      }

      // Render stuffs.
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

      SDL_GL_SwapWindow(win);
    }

    last_time = current_time;

    if (mill_store < RATE_LIMIT && running) {
      SDL_Delay(RATE_LIMIT - mill_store);
    }
  }

  glDeleteProgram(shader_program);
  glDeleteShader(fs);
  glDeleteShader(vs);
  glDeleteBuffers(1, &ebo);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &vao);

  SDL_GL_DeleteContext(gl_context);

  SDL_DestroyWindow(win);
  win = NULL;

  SDL_FreeSurface(surface);
  surface = NULL;
  SDL_FreeSurface(agh);
  agh = NULL;

  IMG_Quit();

  SDL_Quit();
  return EXIT_SUCCESS;
}
