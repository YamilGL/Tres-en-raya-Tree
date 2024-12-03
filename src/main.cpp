#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
//librerias del grafo
#include <vector>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <random>
#include <thread> 
#include <chrono> 
#include <queue> 

#include <map>
#include <set>

using namespace std;
class Arista;

class Nodo {
public:
  vector<Arista*> aristas;
  int id;
  char estado = '\0'; // X o O

  //para graficar
  GLuint VAO,VBO;
  float color[3]; // Componentes RGB del color
  GLuint shaderProgram; // Programa de shaders
  
  float x;
  float y;
  float radius;

  // Constructor
  Nodo(float x, float y, float radius, float r, float g, float b ,int id, char estado='\0') 
    : id(id),x(x), y(y), radius(radius), VAO(0), VBO(0), shaderProgram(0), estado(estado) {
    color[0] = r; color[1] = g; color[2] = b; 
  }

  int utilidad;
  ~Nodo() {
  }
  
};

class Arista {
public:
  Nodo* origen; // Puntero al nodo origen de la arista
  Nodo* destino; // Puntero al nodo destino de la arista
  
  GLuint VAO_line; // ID del Vertex Array Object para la arista
  GLuint VBO_line; // ID del Vertex Buffer Object para la arista
  GLuint shaderProgram; // ID del programa de shaders para la arista
  float color[3]; // Componentes RGB del color de la arista

  // Constructor
  Arista(Nodo* origen, Nodo* destino, float r, float g, float b) :
    origen(origen), destino(destino), VAO_line(0), VBO_line(0), shaderProgram(0) {
    color[0] = r; // Convertir rango de 0-255 a 0.0-1.0
    color[1] = g; // Convertir rango de 0-255 a 0.0-1.0
    color[2] = b; // Convertir rango de 0-255 a 0.0-1.0

    origen->aristas.push_back(this);
    destino->aristas.push_back(this);
  }

  ~Arista() {
  }

};

class Rectangles{
  public:
  float x1,x2,x3,x4,y1,y2,y3,y4,z1,z2,z3,z4;

  unsigned int VAO_rect; // ID del Vertex Array Object para el rect
  unsigned int VBO_rect; // ID del Vertex Buffer Object para el rect
  float color[3]; // Componentes RGB del color del rect

  // Constructor
  Rectangles(float x1, float x2,float x3,float x4,
  float y1, float y2,float y3,float y4,
  float z1, float z2,float z3,float z4,
  float r, float g, float b) :
    x1(x1), x2(x2), x3(x3),x4(x4),
    y1(y1), y2(y2), y3(y3),y4(y4),
    z1(y1), z2(z2), z3(y3),z4(z4),

    VAO_rect(0), VBO_rect(0){
    color[0] = r; // Convertir rango de 0-255 a 0.0-1.0
    color[1] = g; // Convertir rango de 0-255 a 0.0-1.0
    color[2] = b; // Convertir rango de 0-255 a 0.0-1.0
  }
};

class FiguraTablero {
public:
    GLuint shaderProgram;
    float color[3];
    char tipo;
    float x, y, tamanio;
    vector<float> vertices;
    GLuint VAO_line;
    GLuint VBO_line;
    int id = 0;

    FiguraTablero(int id, char tipo, float x, float y, float tamanio, float r, float g, float b) {
      this->id = id;
      this->tipo = tipo;
      this->x = x;
      this->y = y;
      this->tamanio = tamanio;
      this->color[0] = r;
      this->color[1] = g;
      this->color[2] = b;
      this->generarVertices();
    }

    ~FiguraTablero() {
        glDeleteVertexArrays(1, &VAO_line);
        glDeleteBuffers(1, &VBO_line);
        glDeleteProgram(shaderProgram);
    }

    void dibujar() {
        glUseProgram(shaderProgram);
        int figuraColorLocation = glGetUniformLocation(shaderProgram, "figuraColor");
        glUniform3fv(figuraColorLocation, 1, color);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindVertexArray(VAO_line);
        glDrawArrays(GL_LINES, 0, vertices.size());
        glBindVertexArray(0);
    }

    void generarVertices() {
        if (tipo == 'X') {
            vertices = {
                x - tamanio / 2, y - tamanio / 2, 0.0f,
                x + tamanio / 2, y + tamanio / 2, 0.0f,
                x + tamanio / 2, y - tamanio / 2, 0.0f,
                x - tamanio / 2, y + tamanio / 2, 0.0f
            };
        } else {
            const int numVertices = 360;
            vertices.resize(numVertices * 3);
            int index = 0;
            for (int i = 0; i < numVertices; i++) {
                float angle = i * 3.14159 / 180;
                float x1 = x + tamanio / 2 * cos(angle);
                float y1 = y + tamanio / 2 * sin(angle);
                vertices[index++] = x1;
                vertices[index++] = y1;
                vertices[index++] = 0.0f;
            }
        }
    }
};

//compileShaders para nodos
void compileShadersNodo(Nodo& nodo) {
  const char* vertexShaderSource = R"glsl(
  #version 330 core
  layout (location = 0) in vec2 aPos;
  void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
  }
  )glsl";

  const char* fragmentShaderSource = R"glsl(
  #version 330 core
  out vec4 FragColor;
  uniform vec3 nodeColor; // Color del nodo
  void main() {
    FragColor = vec4(nodeColor, 1.0); // Usar el color del nodo
  }
  )glsl";

  GLuint vertexShader, fragmentShader;
  GLint success;
  GLchar infoLog[512];

  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
  }

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
  }

  nodo.shaderProgram = glCreateProgram();
  glAttachShader(nodo.shaderProgram, vertexShader);
  glAttachShader(nodo.shaderProgram, fragmentShader);
  glLinkProgram(nodo.shaderProgram);
  glGetProgramiv(nodo.shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(nodo.shaderProgram, 512, NULL, infoLog);
    cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

//compileShaders para aristas
void compileShadersArista(Arista& arista) {
  const char* vertexShaderSource = R"glsl(
  #version 330 core
  layout (location = 0) in vec2 aPos;
  void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
  }
  )glsl";

  const char* fragmentShaderSource = R"glsl(
  #version 330 core
  out vec4 FragColor;
  uniform vec3 edgeColor; // Color de la arista
  void main() {
    FragColor = vec4(edgeColor, 1.0); // Usar el color de la arista
  }
  )glsl";

  GLuint vertexShader, fragmentShader;
  GLint success;
  GLchar infoLog[512];

  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
  }

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
  }

  arista.shaderProgram = glCreateProgram();
  glAttachShader(arista.shaderProgram, vertexShader);
  glAttachShader(arista.shaderProgram, fragmentShader);
  glLinkProgram(arista.shaderProgram);
  glGetProgramiv(arista.shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(arista.shaderProgram, 512, NULL, infoLog);
    cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

void compileShadersFiguraTablero(FiguraTablero& figura) {
        const char* vertexShaderSource = R"glsl(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            void main() {
                gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
            }
        )glsl";

        const char* fragmentShaderSource = R"glsl(
            #version 330 core
            out vec4 FragColor;
            uniform vec3 figuraColor;
            void main() {
                FragColor = vec4(figuraColor, 1.0);
            }
        )glsl";

        unsigned int vertexShader, fragmentShader;
        int success;
        char infoLog[512];

        // Vertex Shader
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        // Fragment Shader
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        // Shader Program
        figura.shaderProgram = glCreateProgram();
        glAttachShader(figura.shaderProgram, vertexShader);
        glAttachShader(figura.shaderProgram, fragmentShader);
        glLinkProgram(figura.shaderProgram);
        glGetProgramiv(figura.shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(figura.shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Vertex Buffer Object
        glGenVertexArrays(1, &figura.VAO_line);
        glGenBuffers(1, &figura.VBO_line);

        glBindVertexArray(figura.VAO_line);

        glBindBuffer(GL_ARRAY_BUFFER, figura.VBO_line);
        glBufferData(GL_ARRAY_BUFFER, figura.vertices.size() * sizeof(float), figura.vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

void createCircle(Nodo& nodo) {
  glGenVertexArrays(1, &nodo.VAO);
  glGenBuffers(1, &nodo.VBO);

  glBindVertexArray(nodo.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, nodo.VBO);

  float vertices[200];
  float radius = nodo.radius;
  for (int i = 0; i < 100; ++i) {
    float theta = 2.0f * 3.1415926f * float(i) / float(100);
    vertices[i * 2] = radius * cosf(theta) + nodo.x;
    vertices[i * 2 + 1] = radius * sinf(theta) + nodo.y;
  }

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void createEdge(Arista& arista) {
  glGenVertexArrays(1, &(arista.VAO_line));
  glGenBuffers(1, &(arista.VBO_line));

  glBindVertexArray(arista.VAO_line);
  glBindBuffer(GL_ARRAY_BUFFER, arista.VBO_line);

  float lineVertices[] = {
    arista.origen->x, arista.origen->y,
    arista.destino->x, arista.destino->y
  };

  glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void createRectangles(Rectangles& rectangles) {
  float vertices[] = {
    rectangles.x3,rectangles.y3,rectangles.z3,
    rectangles.x4,rectangles.y4,rectangles.z4,
    rectangles.x2,rectangles.y2,rectangles.z2,
    rectangles.x1,rectangles.y1,rectangles.z1,
  };

  glGenVertexArrays(1, &rectangles.VAO_rect);
  glBindVertexArray(rectangles.VAO_rect);

  glGenBuffers(1, &rectangles.VBO_rect);
  glBindBuffer(GL_ARRAY_BUFFER, rectangles.VBO_rect);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

typedef bool (*FuncionDireccion)(Nodo* ori, Nodo* des, int razon, int N);

bool esDiagonal_der(Nodo* ori, Nodo* des, int razon, int N) {
    return ((ori->id + razon + 1) == des->id);
}

bool esDiagonal_izq(Nodo* ori, Nodo* des, int razon, int N) {
    return ((ori->id + N) == des->id);
}

bool esColumna(Nodo* ori, Nodo* des, int razon, int N) {
    return ((ori->id + razon) == des->id && ori->id < N);
}

bool esColumna_(Nodo* ori, Nodo* des, int razon, int N) {
    return ((ori->id + razon) == des->id);
}

bool esFila(Nodo* ori, Nodo* des, int razon, int N) {
    return ((ori->id + 1) == des->id && ori->id % razon == 0);
}

bool esFila_(Nodo* ori, Nodo* des, int razon, int N) {
    return ((ori->id + 1) == des->id);
}

class Tablero{
public:
	vector<Nodo*> nodos;
	vector<Nodo*> nodosJugados;
  int id_centro;
  int id_final;
  bool minmax;
  vector<Arista*> aristas;
  vector<Arista*> aristasJugadas;
  int jugados = 0;
  int N;
  int ia; // turno de IA
  int solucion;

  int alfa = -10000;
  int beta = INT_MAX;

  void getCentro(){
    id_final = nodosJugados[nodosJugados.size()-1]->id;
    
    if(N % 2 == 1) id_centro = id_final/= 2;
    else id_centro = -1;

  }

  void construir(){
    float posx = -0.95f;
    float posy = 0.85f;

    int razon = N+1;
    for (int i = 0; i < (razon * razon); i++){

      if((i+1) % razon != 0 && N*razon>i) nodosJugados.push_back(new Nodo(posx+0.04, posy-0.04, 0.04f,1.0f,0.0f,0.0f,i));      

      if(i==0 || i==N || i==(N*razon)) posx+=0.08;
      else{
        nodos.push_back(new Nodo(posx,posy,0.01f,0.07f,0.13f,0.17f,i));
        posx+=0.08;
      }
      if ((i + 1) % razon == 0) {
        posy -= 0.08;
        posx= -0.95f;
      }
    }

    getCentro();

    for (int i = 0; i < (razon) * (razon); i++) {
      int row = i / razon;
      int col = i % razon;

      //saltar los costados al inicio y final
      if(i>N && i<(razon*N) && i % (razon) < N) agregarArista(i, i + 1, 0.0f,0.0f,1.0f);
      //conectar abajo
      if(i % razon != 0 && (i+1) % razon != 0)  agregarArista(i, i + (razon), 0.0f,0.0f,1.0f);

      // Conectar con nodo a la derecha
      if (col < N) agregarAristaJugada(i, i + 1, 0.07f,0.13f,0.17f);

      // Conectar con nodo abajo
      if (row < N) {
        agregarAristaJugada(i, i + (razon), 0.07f,0.13f,0.17f);
        if(i % (razon + 1) == 0) agregarAristaJugada(i, i + (razon) + 1, 0.07f,0.13f,0.17f);
        if((i + 1) % N == 0)     agregarAristaJugada(i, i + (razon) - 1, 0.07f,0.13f,0.17f);
      }
    }
  }
  
  void eliminarGrafo(){
    for (Nodo* nodo : nodos){
      delete nodo;
    }

    for (Nodo* nod : nodosJugados){
      delete nod;
    }

    for (Arista* arista : aristas)
      delete arista;

    for (Arista* aristas_ : aristasJugadas)
      delete aristas_;

    nodos.clear();
    aristas.clear();
    nodosJugados.clear();
    aristasJugadas.clear();
  }

	Tablero(int n_ = 3) {
    N = n_;
  }

  bool resizeTablero(int n_){
    N = n_;
    if(N<0) {
      cout<<"# negativo";
      return 0;
    }
    eliminarGrafo();
    construir();
    return 1;
  }

	~Tablero() {
    for (Nodo* nodo : nodos)
      delete nodo;

    for (Arista* arista : aristas)
      delete arista;
	}

  void imprimir() {
    // Imprimir los nodos en orden
    for (int i = 0; i < nodos.size(); i++) {
      createCircle(*nodos[i]);
      compileShadersNodo(*nodos[i]);
    }
    // Imprimir las aristas
    for (int i = 0; i < aristas.size(); i++) {
      createEdge(*aristas[i]);
      compileShadersArista(*aristas[i]);
    }

    for (int i = 0; i < aristasJugadas.size(); i++) {
      createEdge(*aristasJugadas[i]);
      compileShadersArista(*aristasJugadas[i]);
    }
  }
  
  Nodo* copiarNodo(Nodo* original) {
    Nodo* copia = new Nodo(original->x,original->y, original->radius, original->color[0], original->color[1], original->color[2], original->id, original->estado);
    return copia;
  }

  // Función para crear una copia profunda del vector nodosJugados
  vector<Nodo*> copiarNodosJugados(const vector<Nodo*>& nodosJugados) {
      vector<Nodo*> copia;
      for (Nodo* nodo : nodosJugados) {
          Nodo* nodoCopia = copiarNodo(nodo);
          copia.push_back(nodoCopia);
      }
      return copia;
  }

  Arista* copiarArista(Arista* original) {
    Arista* copia = new Arista(original->destino, original->destino, original->color[0], original->color[1], original->color[2]);
    return copia;
  }

  // Función para crear una copia profunda del vector nodosJugados
  vector<Arista*> copiarAristasJugados(const vector<Arista*>& aristarJugados) {
      vector<Arista*> copia;
      for (Arista* arista : aristarJugados) {
          Arista* aristaCopia = copiarArista(arista);
          copia.push_back(aristaCopia);
      }
      return copia;
  }

  Nodo* encontrarNodo(int d) {
    for (Nodo* nodo : nodos) {
      if (nodo->id == d) {
        return nodo;
      }
    }
    return NULL;
  }

	void agregarArista(int idOrigen, int idDestino, float r, float g, float b) { // Cambiado a float
    Nodo* origen = nullptr;
    Nodo* destino = nullptr;

    for (Nodo* nodo : nodos) {
      if (nodo->id == idOrigen)
        origen = nodo;
      if (nodo->id == idDestino)
        destino = nodo;
    }

    if (origen && destino) {
      aristas.push_back(new Arista(origen, destino, r, g, b));
    }
  }

  void agregarAristaJugada(int idOrigen, int idDestino, float r, float g, float b) { // Cambiado a float
    Nodo* origen = nullptr;
    Nodo* destino = nullptr;

    for (Nodo* nodo : nodosJugados) {
      if (nodo->id == idOrigen)
        origen = nodo;
      if (nodo->id == idDestino)
        destino = nodo;
    }

    if (origen && destino) {
      aristasJugadas.push_back(new Arista(origen, destino, r, g, b));
    }
  }

  void addAristaJugada(int idOrigen, int idDestino, float r, float g, float b, const vector<Nodo*>& nodosJugados_, vector<Arista*>& aristasJugados_) { // Cambiado a float
    Nodo* origen = nullptr;
    Nodo* destino = nullptr;

    for (Nodo* nodo : nodosJugados_) {
      if (nodo->id == idOrigen)
        origen = nodo;
      if (nodo->id == idDestino)
        destino = nodo;
    }

    if (origen && destino) {
      Arista* x = new Arista(origen, destino, r, g, b);
      aristasJugados_.push_back(x);
    }
  }

  vector<Arista*> addAristasJugadas(const vector<Nodo*>& nodosJugados_){
    vector<Arista*> copia;
    int razon = N+1;
    for (int i = 0; i < (razon) * (razon); i++) {
      int row = i / razon;
      int col = i % razon;

      // Conectar con nodo a la derecha
      if (col < N) addAristaJugada(i, i + 1, 1.0f,0.0f,0.0f, nodosJugados_, copia);

      // Conectar con nodo abajo
      if (row < N) {
        addAristaJugada(i, i + (razon), 1.0f,0.0f,0.0f, nodosJugados_, copia);
        if(i % (razon + 1) == 0) addAristaJugada(i, i + (razon) + 1, 1.0f,0.0f,0.0f, nodosJugados_, copia);
        if((i + 1) % N == 0)     addAristaJugada(i, i + (razon) - 1, 1.0f,0.0f,0.0f, nodosJugados_, copia);
      }
    }
    return copia;
  }

/*
  void profundidadArbol(vector<Nodo*> &temp, char turno){
    int j=0;
    int k=0;

    int alfa_;
    int beta_;

    if(alfa != -10000){
      alfa_ = beta;
    }
    else{
      alfa_ = alfa;
    }

    if(beta == -INT_MAX){
      beta_ = alfa;
    }
    else{
      beta_ = beta;
    }

    turno = (turno == 'X') ? 'O' : 'X';
    vector<Nodo*> actual = copiarNodosJugados(temp);
    for(int i=0;i<actual.size();i++){
      if(!actual[i]->estado) {

        if(alfa != -10000){
          alfa_ = beta;
        }
        else{
          alfa_ = alfa;
        }

        if(beta == -INT_MAX){
          beta_ = alfa;
        }
        else{
          beta_ = beta;
        }
        
        actual[i]->jugadas = copiarNodosJugados(temp);
        actual[i]->jugadas[i]->estado = turno;

        int utilidad = calcularUtilidad(actual[i]->jugadas, turno);
        if(!minmax){ // Minimizar
            beta_ = min(beta_, utilidad);
            if(alfa_ >= beta_) break; // Poda alfa-beta
            if(utilidad == beta_) {
                solucion = actual[i]->jugadas[i]->id;
            }
            alfa = beta_;
        } else { // Maximizar
            alfa_ = max(alfa_, utilidad);
            if(alfa_ >= beta_) break; // Poda alfa-beta
            if(utilidad == alfa_) {
                solucion = actual[i]->jugadas[i]->id;
            }
            beta = alfa_;
        }
      }
    }
    cout<<"Jugada J: "<<j<<"Turno: "<<turno<<endl;
    cout<<endl;
  }
*/
  bool verificarContinuidad(Nodo* ori, Nodo* des, int razon, int N, int& cont, int& utilidadX, int& utilidadO, Arista* aristas_, FuncionDireccion direccion) {
    char estado = ori->estado == '\0' ? des->estado : ori->estado;
    int c = 0;
    if (estado != '\0') {
      c++;
    }
    cont++;

    if (ori->estado != des->estado && (des->estado != '\0') && ori->estado != '\0') {
      return false;
    }

    for (int k = 1; k < N - 1; k++) {

      ori = des;
      for (int l = 0; l < ori->aristas.size(); l++) {
        des = (ori->aristas[l]->origen == ori) ? ori->aristas[l]->destino : ori->aristas[l]->origen;
        if (direccion(ori, des, razon, N)) {
          if ((estado != '\0' && estado == des->estado && c==0)) {
            return false;
          }
          char estadoActual = des->estado == '\0' ? ori->estado : des->estado;
          if (c == 0 && estadoActual != '\0') {
            estado = estadoActual;
            c++;
          }
          if (estadoActual == estado || estadoActual == '\0') {
            cont++;
            break;
          } else if (estadoActual != '\0' && estadoActual != estado) {
            break;
          }
        }
      }
    }

    if (cont == (N - 1)) {
      if (estado == 'X') {
        utilidadX++;
      } else if (estado == 'O') {
        utilidadO++;
      }
      return true;
    }
    return false;
  }


  bool verificarContinuidadGanadora(Nodo* ori, Nodo* des, int razon, int N, FuncionDireccion direccion) {
    char estado = '\0'; 
    int contX = 0; 
    int contO = 0;

    for (int k = 1; k < N - 1; k++) {
        ori = des;
        for (int l = 0; l < ori->aristas.size(); l++) {
            des = (ori->aristas[l]->origen == ori) ? ori->aristas[l]->destino : ori->aristas[l]->origen;
            if (direccion(ori, des, razon, N)) {
                if(des->estado == 'X') contX++;
                if(des->estado == 'O') contO++;
            }
        }
    }

      if(contX>=1 && contO>=1){
        return false;
      }
      else if(contX>=1 && contO == 0){
        return true;
      }
      else if(contO>=1 && contX == 0){
        return true;
      }
    return false;
  }



  int calcularUtilidad(vector<Nodo*> &actual, char turno){
    int razon = N + 1;
    int cont = 0;

    int utilidadX=0;
    int utilidadO=0;

    vector<Nodo*> util = copiarNodosJugados(actual);
    vector<Arista*> utilAristas = addAristasJugadas(util);

    for (Arista* arista : utilAristas) {
      Nodo* ori = arista->origen;
      Nodo* des = arista->destino;
      cont = 0;

      if (arista->color[1] != 1.0f) {
        if (esDiagonal_der(ori, des, razon, N)) {
          arista->color[1] = 1.0f;
          if (verificarContinuidad(ori, des, razon, N, cont, utilidadX, utilidadO, arista, esDiagonal_der)) {
              cout << "-> "<< "Diagonal" << "\t";
          }
        }
        ori = arista->origen;
        des = arista->destino;
        if (esDiagonal_izq(ori, des, razon, N)) {
          arista->color[1] = 1.0f;
          if (verificarContinuidad(ori, des, razon, N, cont, utilidadX, utilidadO, arista, esDiagonal_izq)) {
              cout << "-> "<< "Diagonal" << "\t";
          }
        }

      ori = arista->origen;
      des = arista->destino;
      if (esColumna(ori, des, razon, N)) {
          arista->color[1] = 1.0f;
          if (verificarContinuidad(ori, des, razon, N, cont, utilidadX, utilidadO, arista, esColumna_)) {
              cout << "-> "<< "Columnas" << "\t";
          }
      }

      ori = arista->origen;
      des = arista->destino;
      if (esFila(ori, des, razon, N)) {
          arista->color[1] = 1.0f;
          if (verificarContinuidad(ori, des, razon, N, cont, utilidadX, utilidadO, arista, esFila_)) {
              cout << "-> "<< "Filas" << "\t";
          }
        }
      }
    }
    cout<<endl;
    cout<<"Utilidad X: "<<utilidadX<<"\t";
    cout<<"Utilidad O: "<<utilidadO<<endl;
    if(ia == 'X'){
      return utilidadX - utilidadO;
    }
    else return utilidadO - utilidadX;
  }

  bool comprobarVictoria(){
    int razon = N + 1;
    int cont = 0;

    for (Arista* arista : aristasJugadas) {
      Nodo* ori = arista->origen;
      Nodo* des = arista->destino;
      cont = 0;

      if (esDiagonal_der(ori, des, razon, N)) {
        if (verificarContinuidadGanadora(ori, des, razon, N, esDiagonal_der)) {
            arista->color[0] = 1.0f;
            arista->color[1] = 0.0f;
            arista->color[2] = 0.0f;
            return 1;
        }
      }
      
      ori = arista->origen;
      des = arista->destino;

      if (esDiagonal_izq(ori, des, razon, N)) {
        if (verificarContinuidadGanadora(ori, des, razon, N, esDiagonal_izq)) {
            arista->color[0] = 1.0f;
            arista->color[1] = 0.0f;
            arista->color[2] = 0.0f;
            return 1;
        }
      }

    ori = arista->origen;
    des = arista->destino;
    if (esColumna(ori, des, razon, N)) {
        if (verificarContinuidadGanadora(ori, des, razon, N, esColumna_)) {
            arista->color[0] = 1.0f;
            arista->color[1] = 0.0f;
            arista->color[2] = 0.0f;
            return 1;
        }
    }

    ori = arista->origen;
    des = arista->destino;
    if (esFila(ori, des, razon, N)) {
        if (verificarContinuidadGanadora(ori, des, razon, N, esFila_)) {
            arista->color[0] = 1.0f;
            arista->color[1] = 0.0f;
            arista->color[2] = 0.0f;
            return 1;
        }
      }
    }
    return 0;
  }

/*
  int calcular(int profundidad, char turno_){
    int j;
    minmax = (profundidad % 2 == 0) ? 1 : 0;
    ia = turno_;

    vector<Nodo*> temp = copiarNodosJugados(nodosJugados);
    vector<vector<Nodo*>> depth;  depth.push_back(temp);

    vector<vector<Nodo*>> road; 

    for(int k=1; k<profundidad-2; k++){ 
      for(int i=0; i<temp.size(); i++){
        if(!temp[i]->estado) {
          temp[i]->jugadas = copiarNodosJugados(temp);
          temp[i]->jugadas[i]->estado = turno_;
          depth.insert(depth.begin(),temp[i]->jugadas);
          temp = temp[i]->jugadas;
          turno_ = (turno_ == 'X') ? 'O' : 'X';
          break;
        }
      }
      cout<<endl;
    }
    
    cout<<"Beta: "<<beta<<endl;
    cout<<"Alfa: "<<alfa<<endl;
    cout<<"Turno: "<<turno_<<"\t"<<"MinMax: "<<minmax<<endl;

    
    cout<<"------------------------------------------------------------------------"<<endl;
    vector<Nodo*> actual = depth.front();
    depth.erase(depth.begin());
    vector<Nodo*> temp_ = copiarNodosJugados(actual);

    for(int i=0; i<actual.size(); i++){
        if(!actual[i]->estado) {
          actual[i]->jugadas = copiarNodosJugados(temp_);
          actual[i]->jugadas[i]->estado = turno_;
          profundidadArbol(actual[i]->jugadas,turno_);
          cout<<"Beta: "<<beta<<endl;
          cout<<"Alfa: "<<alfa<<endl;
          cout<<"Turno: "<<turno_<<"\t"<<"MinMax: "<<minmax<<"\t"<<"I: "<<i<<endl;
          cout<<"ID Arbol: "<<actual[i]->id<<"\t";
        }
      }
      
      minmax = !minmax;
      turno_ = (turno_ == 'X') ? 'O' : 'X';

    // 1 = Max , 0 = Min

    while (!depth.empty()) {
      cout<<"------------------------------------------------------------------------"<<endl;
      vector<Nodo*> actual = depth.front();
      depth.erase(depth.begin());
      vector<Nodo*> temp_ = copiarNodosJugados(actual);
      int c = 0;

      //p = 5, minmax = 0, min

      // navegar con conexion a los hijos, sacar el resultado actualizando al HEad
      for(int i=0; i<actual.size(); i++){
        if(!actual[i]->estado && c > 0) {
          actual[i]->jugadas = copiarNodosJugados(temp_);
          actual[i]->jugadas[i]->estado = turno_;
          profundidadArbol(actual[i]->jugadas,turno_);
          cout<<"Beta: "<<beta<<endl;
          cout<<"Alfa: "<<alfa<<endl;
          cout<<"Turno: "<<turno_<<"\t"<<"MinMax: "<<minmax<<"\t"<<"I: "<<i<<endl;
          cout<<"ID Arbol: "<<actual[i]->id<<"\t";
        }
        if(!actual[i]->estado) c++;
      }
      cout<<endl;

      minmax = !minmax;
      turno_ = (turno_ == 'X') ? 'O' : 'X';
    }
    return solucion;
  }*/
};

class Conexion;

class Jugada{
  public:
  vector<Nodo*> nodos;
  vector<Arista*> aristas;

  vector<Conexion*> conexion;

  bool poda = 0;

  int alfa = INT_MIN;
  int beta = INT_MAX;
  int utilidad;
  int jugadaID = -1;

  int N;
  char ia; // turno de IA

  // Constructor
  Jugada(int N, char ia, vector<Nodo*> nodos_, vector<Arista*> aristas_, int alfa_padre, int beta_padre) 
    : N(N),ia(ia){
      nodos = copiarNodosJugados(nodos_);
      aristas = addAristasJugadas(nodos);
      beta = beta_padre;
      alfa = alfa_padre;
  }

  ~Jugada() {
    for (Nodo* nodo : nodos){
      nodo->aristas.clear();
      delete nodo;
    }
    for (Arista* arista : aristas)
      delete arista;

    nodos.clear();
    aristas.clear();
  }

  
  Nodo* copiarNodo(Nodo* original) {
    Nodo* copia = new Nodo(original->x,original->y, original->radius, original->color[0], original->color[1], original->color[2], original->id, original->estado);
    return copia;
  }

  // Función para crear una copia profunda del vector nodosJugados
  vector<Nodo*> copiarNodosJugados(const vector<Nodo*>& nodosJugados) {
      vector<Nodo*> copia;
      for (Nodo* nodo : nodosJugados) {
          Nodo* nodoCopia = copiarNodo(nodo);
          copia.push_back(nodoCopia);
      }
      return copia;
  }

  Arista* copiarArista(Arista* original) {
    Arista* copia = new Arista(original->origen, original->destino, original->color[0], original->color[1], original->color[2]);
    return copia;
  }

  // Función para crear una copia profunda del vector nodosJugados
  vector<Arista*> copiarAristasJugados(const vector<Arista*>& aristarJugados) {
      vector<Arista*> copia;
      for (Arista* arista : aristarJugados) {
          Arista* aristaCopia = copiarArista(arista);
          copia.push_back(aristaCopia);
      }
      return copia;
  }

    void addAristaJugada(int idOrigen, int idDestino, float r, float g, float b, const vector<Nodo*>& nodosJugados_, vector<Arista*>& aristasJugados_) { // Cambiado a float
    Nodo* origen = nullptr;
    Nodo* destino = nullptr;

    for (Nodo* nodo : nodosJugados_) {
      if (nodo->id == idOrigen)
        origen = nodo;
      if (nodo->id == idDestino)
        destino = nodo;
    }

    if (origen && destino) {
      Arista* x = new Arista(origen, destino, r, g, b);
      aristasJugados_.push_back(x);
    }
  }

  vector<Arista*> addAristasJugadas(const vector<Nodo*>& nodosJugados_){
    vector<Arista*> copia;
    int razon = N+1;
    for (int i = 0; i < (razon) * (razon); i++) {
      int row = i / razon;
      int col = i % razon;

      // Conectar con nodo a la derecha
      if (col < N) addAristaJugada(i, i + 1, 1.0f,0.0f,0.0f, nodosJugados_, copia);

      // Conectar con nodo abajo
      if (row < N) {
        addAristaJugada(i, i + (razon), 1.0f,0.0f,0.0f, nodosJugados_, copia);
        if(i % (razon + 1) == 0) addAristaJugada(i, i + (razon) + 1, 1.0f,0.0f,0.0f, nodosJugados_, copia);
        if((i + 1) % N == 0)     addAristaJugada(i, i + (razon) - 1, 1.0f,0.0f,0.0f, nodosJugados_, copia);
      }
    }
    return copia;
  }
/*
  void profundidadArbol(vector<Nodo*> &temp, char turno){
    int j=0;
    int k=0;

    int alfa_;
    int beta_;

    if(alfa != -10000){
      alfa_ = beta;
    }
    else{
      alfa_ = alfa;
    }

    if(beta == -INT_MAX){
      beta_ = alfa;
    }
    else{
      beta_ = beta;
    }

    turno = (turno == 'X') ? 'O' : 'X';
    vector<Nodo*> actual = copiarNodosJugados(temp);
    for(int i=0;i<actual.size();i++){
      if(!actual[i]->estado) {

        if(alfa != -10000){
          alfa_ = beta;
        }
        else{
          alfa_ = alfa;
        }

        if(beta == -INT_MAX){
          beta_ = alfa;
        }
        else{
          beta_ = beta;
        }
        
        actual[i]->jugadas = copiarNodosJugados(temp);
        actual[i]->jugadas[i]->estado = turno;

        int utilidad = calcularUtilidad(actual[i]->jugadas, turno);
        if(!minmax){ // Minimizar
            beta_ = min(beta_, utilidad);
            if(alfa_ >= beta_) break; // Poda alfa-beta
            if(utilidad == beta_) {
                solucion = actual[i]->jugadas[i]->id;
            }
            alfa = beta_;
        } else { // Maximizar
            alfa_ = max(alfa_, utilidad);
            if(alfa_ >= beta_) break; // Poda alfa-beta
            if(utilidad == alfa_) {
                solucion = actual[i]->jugadas[i]->id;
            }
            beta = alfa_;
        }
      }
    }
    cout<<"Jugada J: "<<j<<"Turno: "<<turno<<endl;
    cout<<endl;
  }

  bool verificarContinuidad(Nodo* ori, Nodo* des, int razon, int N, int& cont, int& utilidadX, int& utilidadO, Arista* aristas_, FuncionDireccion direccion) {
    char estado = ori->estado == '\0' ? des->estado : ori->estado;
    int c = 0;
    if (estado != '\0') {
      c++;
    }
    cont++;

    if (ori->estado != des->estado && (des->estado != '\0') && ori->estado != '\0') {
      return false;
    }

    for (int k = 1; k < N - 1; k++) {

      ori = des;
      for (int l = 0; l < ori->aristas.size(); l++) {
        des = (ori->aristas[l]->origen == ori) ? ori->aristas[l]->destino : ori->aristas[l]->origen;
        if (direccion(ori, des, razon, N)) {
          if ((estado != '\0' && estado == des->estado && c==0)) {
            return false;
          }
          char estadoActual = des->estado == '\0' ? ori->estado : des->estado;
          if (c == 0 && estadoActual != '\0') {
            estado = estadoActual;
            c++;
          }
          if (estadoActual == estado || estadoActual == '\0') {
            cont++;
            break;
          } else if (estadoActual != '\0' && estadoActual != estado) {
            break;
          }
        }
      }
    }

    if (cont == (N - 1)) {
      if (estado == 'X') {
        utilidadX++;
        cout << "X";
      } else if (estado == 'O') {
        utilidadO++;
        cout << "O";
      }
      return true;
    }
    return false;
  }

  bool verificarContinuidadGanadora(Nodo* ori, Nodo* des, int razon, int N, FuncionDireccion direccion) {
    if (ori->estado != des->estado) return false;
    if (ori->estado == '\0' || des->estado == '\0') return false;

    for (int k = 1; k < N - 1; k++) {
      ori = des;
      for (int l = 0; l < ori->aristas.size(); l++) {
        des = (ori->aristas[l]->origen == ori) ? ori->aristas[l]->destino : ori->aristas[l]->origen;
        if (direccion(ori, des, razon, N)) {
          if (ori->estado != des->estado) {
            return false;
          }
          ori->aristas[l]->color[0] = 1.0f;
          ori->aristas[l]->color[1] = 0.0f;
          ori->aristas[l]->color[2] = 0.0f;
        }
      }
    }
    return true;
  }

  int calcularUtilidad(vector<Nodo*> &actual, char turno){
    int razon = N + 1;
    int cont = 0;

    int utilidadX=0;
    int utilidadO=0;

    vector<Nodo*> util = copiarNodosJugados(actual);
    vector<Arista*> utilAristas = addAristasJugadas(util);

    for (Arista* arista : utilAristas) {
      Nodo* ori = arista->origen;
      Nodo* des = arista->destino;
      cont = 0;

      if (arista->color[1] != 1.0f) {
        if (esDiagonal_der(ori, des, razon, N)) {
          arista->color[1] = 1.0f;
          if (verificarContinuidad(ori, des, razon, N, cont, utilidadX, utilidadO, arista, esDiagonal_der)) {
              cout << "-> "<< "Diagonal" << "\t";
          }
        }
        if (esDiagonal_izq(ori, des, razon, N)) {
          arista->color[1] = 1.0f;
          if (verificarContinuidad(ori, des, razon, N, cont, utilidadX, utilidadO, arista, esDiagonal_izq)) {
              cout << "-> "<< "Diagonal" << "\t";
          }
        }

      ori = arista->origen;
      des = arista->destino;
      if (esColumna(ori, des, razon, N)) {
          arista->color[1] = 1.0f;
          if (verificarContinuidad(ori, des, razon, N, cont, utilidadX, utilidadO, arista, esColumna_)) {
              cout << "-> "<< "Columnas" << "\t";
          }
      }

      ori = arista->origen;
      des = arista->destino;
      if (esFila(ori, des, razon, N)) {
          arista->color[1] = 1.0f;
          if (verificarContinuidad(ori, des, razon, N, cont, utilidadX, utilidadO, arista, esFila_)) {
              cout << "-> "<< "Filas" << "\t";
          }
        }
      }
    }
    cout<<endl;
    cout<<"Utilidad X: "<<utilidadX<<"\t";
    cout<<"Utilidad O: "<<utilidadO<<endl;
    if(ia == 'X'){
      return utilidadX - utilidadO;
    }
    else return utilidadO - utilidadX;
  }

  bool comprobarVictoria(){
    int razon = N + 1;
    int cont = 0;

    for (Arista* arista : aristasJugadas) {
      Nodo* ori = arista->origen;
      Nodo* des = arista->destino;
      cont = 0;

      if (esDiagonal_der(ori, des, razon, N)) {
        if (verificarContinuidadGanadora(ori, des, razon, N, esDiagonal_der)) {
            arista->color[0] = 1.0f;
            arista->color[1] = 0.0f;
            arista->color[2] = 0.0f;
            return 1;
        }
      }
      if (esDiagonal_izq(ori, des, razon, N)) {
        if (verificarContinuidadGanadora(ori, des, razon, N, esDiagonal_izq)) {
            arista->color[0] = 1.0f;
            arista->color[1] = 0.0f;
            arista->color[2] = 0.0f;
            return 1;
        }
      }

    ori = arista->origen;
    des = arista->destino;
    if (esColumna(ori, des, razon, N)) {
        if (verificarContinuidadGanadora(ori, des, razon, N, esColumna_)) {
            arista->color[0] = 1.0f;
            arista->color[1] = 0.0f;
            arista->color[2] = 0.0f;
            return 1;
        }
    }

    ori = arista->origen;
    des = arista->destino;
    if (esFila(ori, des, razon, N)) {
        if (verificarContinuidadGanadora(ori, des, razon, N, esFila_)) {
            arista->color[0] = 1.0f;
            arista->color[1] = 0.0f;
            arista->color[2] = 0.0f;
            return 1;
        }
      }
    }
    return 0;
  }

  int calcular(int profundidad, char turno_){
    int j;
    minmax = (profundidad % 2 == 0) ? 1 : 0;
    ia = turno_;

    vector<Nodo*> temp = copiarNodosJugados(nodosJugados);
    vector<vector<Nodo*>> depth;  depth.push_back(temp);

    vector<vector<Nodo*>> road; 

    for(int k=1; k<profundidad-2; k++){ 
      for(int i=0; i<temp.size(); i++){
        if(!temp[i]->estado) {
          temp[i]->jugadas = copiarNodosJugados(temp);
          temp[i]->jugadas[i]->estado = turno_;
          depth.insert(depth.begin(),temp[i]->jugadas);
          temp = temp[i]->jugadas;
          turno_ = (turno_ == 'X') ? 'O' : 'X';
          break;
        }
      }
      cout<<endl;
    }
    
    cout<<"Beta: "<<beta<<endl;
    cout<<"Alfa: "<<alfa<<endl;
    cout<<"Turno: "<<turno_<<"\t"<<"MinMax: "<<minmax<<endl;

    
    cout<<"------------------------------------------------------------------------"<<endl;
    vector<Nodo*> actual = depth.front();
    depth.erase(depth.begin());
    vector<Nodo*> temp_ = copiarNodosJugados(actual);

    for(int i=0; i<actual.size(); i++){
        if(!actual[i]->estado) {
          actual[i]->jugadas = copiarNodosJugados(temp_);
          actual[i]->jugadas[i]->estado = turno_;
          profundidadArbol(actual[i]->jugadas,turno_);
          cout<<"Beta: "<<beta<<endl;
          cout<<"Alfa: "<<alfa<<endl;
          cout<<"Turno: "<<turno_<<"\t"<<"MinMax: "<<minmax<<"\t"<<"I: "<<i<<endl;
          cout<<"ID Arbol: "<<actual[i]->id<<"\t";
        }
      }
      
      minmax = !minmax;
      turno_ = (turno_ == 'X') ? 'O' : 'X';

    // 1 = Max , 0 = Min

    while (!depth.empty()) {
      cout<<"------------------------------------------------------------------------"<<endl;
      vector<Nodo*> actual = depth.front();
      depth.erase(depth.begin());
      vector<Nodo*> temp_ = copiarNodosJugados(actual);
      int c = 0;

      //p = 5, minmax = 0, min

      // navegar con conexion a los hijos, sacar el resultado actualizando al HEad
      for(int i=0; i<actual.size(); i++){
        if(!actual[i]->estado && c > 0) {
          actual[i]->jugadas = copiarNodosJugados(temp_);
          actual[i]->jugadas[i]->estado = turno_;
          profundidadArbol(actual[i]->jugadas,turno_);
          cout<<"Beta: "<<beta<<endl;
          cout<<"Alfa: "<<alfa<<endl;
          cout<<"Turno: "<<turno_<<"\t"<<"MinMax: "<<minmax<<"\t"<<"I: "<<i<<endl;
          cout<<"ID Arbol: "<<actual[i]->id<<"\t";
        }
        if(!actual[i]->estado) c++;
      }
      cout<<endl;

      minmax = !minmax;
      turno_ = (turno_ == 'X') ? 'O' : 'X';
    }
    return solucion;
  }

*/
};

class Conexion{
  public:
  Jugada* padre;
  Jugada* hijo;

  
  Conexion(Jugada* padre, Jugada* hijo) :
    padre(padre), hijo(hijo) {
    padre->conexion.push_back(this);
    hijo->conexion.push_back(this);
  }

  ~Conexion() {
  }
};

// variables globales
Tablero T;
int n;
char turno = 'X';
char ia;
int profundidad;
int fichasJugadas = 0;
vector<FiguraTablero> figuras;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}
void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}
/*
int profundidadasd(int nodoInicial, int nodoFinal) {

  while (!road.empty()) {
    Nodo* nodoActual = road.front();
    road.erase(road.begin());
    
    nodoActual->color[0] = 255.0f;
    nodoActual->color[1] = 0;
    nodoActual->color[2] = 0;

    cout<<"ID: "<<nodoActual->id<<endl;
    if (nodoActual->id == nodoFinal) {
      cout << "Done" << endl;
      return recorrido;
    }

    for (int i = 0; i < nodoActual->aristas.size(); i++) {
      Nodo* siguiente = (nodoActual->aristas[i]->origen == nodoActual) ? nodoActual->aristas[i]->destino : nodoActual->aristas[i]->origen;
      if (siguiente->color[0] == 0.0f || siguiente->color[0] == 1.0f) {
        siguiente->color[0] = 0.0f;
        siguiente->color[1] = 0.0f;
        siguiente->color[2] = 255.0f;
        siguiente->distancia = nodoActual->id;
        road.insert(road.begin(),siguiente);
        recorrido += aristas[i]->peso;
        cout<<"Siguiente: " << road.front()->id<<"\t";
      }
        
    }
    cout<<endl;
  }

  cout << "No path found" << endl;
  return recorrido;
}
*/

int calcularUtilidad(Jugada* actual){
  int razon = n + 1;
  int cont = 0;

  int utilidadX=0;
  int utilidadO=0;

  for (Arista* arista : actual->aristas) {
    Nodo* ori = arista->origen;
    Nodo* des = arista->destino;
    cont = 0;

    if (esDiagonal_der(ori, des, razon, n)) {
      T.verificarContinuidad(ori, des, razon, n, cont, utilidadX, utilidadO, arista, esDiagonal_der);
    }
    
  ori = arista->origen;
  des = arista->destino;
    if (esDiagonal_izq(ori, des, razon, n)) {
      T.verificarContinuidad(ori, des, razon, n, cont, utilidadX, utilidadO, arista, esDiagonal_izq);
    }

  ori = arista->origen;
  des = arista->destino;
  if (esColumna(ori, des, razon, n)) {
      T.verificarContinuidad(ori, des, razon, n, cont, utilidadX, utilidadO, arista, esColumna_);
  }

  ori = arista->origen;
  des = arista->destino;
  if (esFila(ori, des, razon, n)) {
      T.verificarContinuidad(ori, des, razon, n, cont, utilidadX, utilidadO, arista, esFila_);
    }
  }
  if(ia == 'X'){
    return utilidadX - utilidadO;
  }
  else return utilidadO - utilidadX;
}

void arbol(Jugada* &padre, int profundidad_estado, bool minmax, char turno_actual, bool poda){
  int mejorLocalID = -1;

  if(profundidad_estado == 0) {
    padre->utilidad = calcularUtilidad(padre);
    if(minmax){
      padre->beta = padre->utilidad;
    }
    else{
      padre->alfa = padre->utilidad;
    }
    return;
  }
  for(int i = 0; i<padre->nodos.size() && !padre->poda; i++){
    if(!padre->nodos[i]->estado) {
      Jugada* hijo = new Jugada (n, turno, padre->nodos, padre->aristas, padre->alfa, padre->beta);
      hijo->nodos[i]->estado = turno_actual;
      Conexion(padre, hijo);

      arbol(hijo, profundidad_estado - 1, !minmax, (turno_actual == 'X') ? 'O' : 'X', padre->poda);

      if(!minmax && hijo->utilidad < padre->beta) { // Minimizar
        padre->beta = hijo->utilidad;
        mejorLocalID = i; // Actualizar jugadaID solo si se encuentra una mejor utilidad
      } else if(minmax && hijo->utilidad > padre->alfa) { // Maximizar
        padre->alfa = hijo->utilidad;
        mejorLocalID = i; // Actualizar jugadaID solo si se encuentra una mejor utilidad
      }
      
      if(padre->alfa >= padre->beta) {
        padre->poda = 1;
        break; // Salir del bucle porque se ha encontrado una poda.
      }
    } 
  }

  if(mejorLocalID != -1 && !padre->poda) {
    padre->jugadaID = mejorLocalID; // Propagar la mejor jugada hacia arriba si se encontró una
  }
  padre->utilidad = (minmax) ? padre->alfa : padre->beta;
}

int jugadaIA(){
  Jugada* inicial = new Jugada(n, turno, T.nodosJugados, T.aristasJugadas, INT_MIN, INT_MAX);
  int mejorMovimiento = -1; // ID del mejor movimiento
  int max_prof_posible = (n * n) - fichasJugadas;
  max_prof_posible = min(max_prof_posible,profundidad);

  arbol(inicial, max_prof_posible, 1, turno, 0);

  mejorMovimiento = inicial->jugadaID;
  cout<<"Utilidad Raiz: "<<inicial->utilidad<<endl;

  delete inicial; // Liberar memoria
  return mejorMovimiento;
}

void resetGrafo (Tablero T) {
  for(FiguraTablero fig: figuras){
    fig.color[0] = 0.07f;
    fig.color[1] = 0.13f;
    fig.color[2] = 0.17f;
    fig.tipo = '\0';
  }
  figuras.clear();
}

void turnoIA(){

}

void jugadaHumano(Nodo* actual){
  if(!actual->estado){
    actual->estado = turno;
    // Dibujar la X en el centro de la ventana
    float r,g,b;
    if (turno == 'X') {
        r = 1.0f; g = 0.0f; b = 0.0f; // Rojo
    } else {
        r = 0.0f; g = 0.0f; b = 1.0f; // Azul
    }

    figuras.emplace_back(actual->id, turno, actual->x, actual->y, 0.05f,r,g,b);
    for (int i = 0; i < figuras.size(); i++) compileShadersFiguraTablero(figuras[i]);
  }
  else{
    cout<<"Elija otro casillero"<<endl;
  }
}
// Función para manejar los eventos de click
void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
  // Verificar si el botón izquierdo del mouse ha sido presionado
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Imprimir las coordenadas del click
    // cout << "Click en coordenadas (" << xpos << ", " << ypos << ")" << endl;
    // Obtener las coordenadas normalizadas del cursor del mouse
    float normalizedX = (xpos / 800) * 2 - 1; // Convertir de rango [0, screenWidth] a [-1, 1]
    float normalizedY =  1 - ((ypos/800)*2); // Convertir de rango [0, screenHeight] a [-1, 1]
    // cout << "Click en coordenadas normalizadas (" << normalizedX << ", " << normalizedY << ")" << endl;

    // cout << "Coordenadas del primer Nodo(" << G.nodos[0]->x << ", " << G.nodos[0]->y << ")" << endl;
    for (int i = 0; i<T.nodosJugados.size(); i++) {
      if (pow(normalizedX - T.nodosJugados[i]->x, 2) + pow(normalizedY - T.nodosJugados[i]->y, 2) <= pow(T.nodosJugados[i]->radius, 2)) {
        // Cambiar el color del nodo encontrado a rojo
        if(fichasJugadas == 9) break;
        jugadaHumano(T.nodosJugados[i]);
        fichasJugadas++;
        turno = (turno == 'X') ? 'O' : 'X';
        ia = turno;
        int solucion = jugadaIA();
        if(solucion != -1) jugadaHumano(T.nodosJugados[solucion]);
        else cout<<"No se puede mover"<<endl;
        fichasJugadas++;
        turno = (turno == 'X') ? 'O' : 'X';
        //T.comprobarVictoria();
        break; // Salir del bucle al encontrar el nodo
      }
  }
  if (normalizedX > 0.7f && normalizedX <0.99f  && normalizedY < 0.88f && normalizedY > 0.75f)  {           
    int solucion = jugadaIA();
    if(solucion != -1) jugadaHumano(T.nodosJugados[solucion]);
    else cout<<"No se puede mover"<<endl;
    fichasJugadas++;
    turno = (turno == 'X') ? 'O' : 'X';
  }
  // cout << "Coordenadas del primer Nodo(" << G.nodos[0]->x << ", " << G.nodos[0]->y << ")" << endl;
  if (normalizedX > 0.7f && normalizedX <0.99f  && normalizedY < 0.68f && normalizedY > 0.55f){           
    cout<<endl<<"Ingrese nuevo N: ";cin>>n;
    bool tsd = T.resizeTablero(n);
    resetGrafo(T);
    T.imprimir();
  }
  }
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(800, 800, "Grafo", NULL, NULL);
  if (window == NULL) {
    cout << "Failed to create GLFW Window" << endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


  //funcion del mouse
  glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
    mouse_callback(window, button, action, mods);
  });

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    cout << "Failed to initialize GLAD" << endl;
    return -1;
  }
  //termina de crear la pantalla 

  //glViewport(0, 0, 800, 800);
  //glClearColor(0.07f, 0.13f, 0.17f, 1.0f);

  //main del grafo
  cout << endl;
  //cout << (G.nodos[0])->aristas[0]->peso;
  cout<<"N: ";cin>>n;
  cout<<"Profundidad: ";cin>>profundidad;cout<<endl;
  T.resizeTablero(n);
  T.imprimir();

  vector<Rectangles*> algoritmos_mostrar;
  float x1,x2,x3,x4,y1,y2,y3,y4,z1,z2,z3,z4;
  x1 =0.7f;
  y1 = 0.88f;
  z1 = 0.0f;

  x2 = 0.99f;
  y2 = 0.88f;
  z2 = 0.0f;

  x3 = 0.7f;
  y3 = 0.75f;
  z3 = 0.0f;

  x4 =  0.99f;
  y4 = 0.75f;
  z4 = 0.0f;

  for(int i = 0; i <2;i++){
    algoritmos_mostrar.push_back(new Rectangles(x1,x2,x3,x4,y1,y2,y3,y4,z1,z2,z3,z4,0,255,0));
    createRectangles(*algoritmos_mostrar[i]);
    y1-=0.2f;
    y2-=0.2f;
    y3-=0.2f;
    y4-=0.2f;
  }

  while (!glfwWindowShouldClose(window)) {
    // Procesar eventos de la ventana
    glfwPollEvents();
    processInput(window);

    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    
    for (int i = 0; i < T.aristasJugadas.size(); i++) {
      glUseProgram(T.aristasJugadas[i]->shaderProgram);
      int edgeColorLocation = glGetUniformLocation(T.aristasJugadas[i]->shaderProgram, "edgeColor");
      glUniform3fv(edgeColorLocation, 1, T.aristasJugadas[i]->color);
      glBindVertexArray(T.aristasJugadas[i]->VAO_line);
      glDrawArrays(GL_LINES, 0, 2);
    }

    for (int i = 0; i < figuras.size(); i++) {
      figuras[i].dibujar();
    }

    // Imprimir los nodos en orden
    for (int i = 0; i < T.nodos.size(); i++) {
      //cout << G.nodos[i]->id << "\t";
      glUseProgram(T.nodos[i]->shaderProgram);
      // Pasa el color del nodo al shader fragment
      int nodeColorLocation = glGetUniformLocation(T.nodos[i]->shaderProgram, "nodeColor");
      glUniform3fv(nodeColorLocation, 1, T.nodos[i]->color);
      glBindVertexArray(T.nodos[i]->VAO);
      glDrawArrays(GL_TRIANGLE_FAN, 0, 100);
    }
    // Dibujar aristas
    // Imprimir las aristas
    for (int i = 0; i < T.aristas.size(); i++) {
      glUseProgram(T.aristas[i]->shaderProgram);
      int edgeColorLocation = glGetUniformLocation(T.aristas[i]->shaderProgram, "edgeColor");
      glUniform3fv(edgeColorLocation, 1, T.aristas[i]->color);
      glBindVertexArray(T.aristas[i]->VAO_line);
      glDrawArrays(GL_LINES, 0, 2);
    }

  
    for(int i = 0; i<algoritmos_mostrar.size();i++)    {
      glBindVertexArray(algoritmos_mostrar[i]->VAO_rect);
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Modificado para dibujar un rectángulo
    }

    //dibuja rectangulos
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  //liberar memoria
  for (int i = 0; i < T.nodos.size(); i++) {
    glDeleteVertexArrays(1, &(T.nodos[i])->VAO);
    glDeleteBuffers(1, &(T.nodos[i])->VBO);
    glDeleteProgram(T.nodos[i]->shaderProgram);
  }
  for (int i = 0; i < T.aristas.size(); i++) {
    glDeleteVertexArrays(1, &(T.aristas[i])->VAO_line);
    glDeleteBuffers(1, &(T.aristas[i])->VBO_line);
    glDeleteProgram(T.aristas[i]->shaderProgram);
  }
  for (int i = 0; i < figuras.size(); i++) {
    glDeleteVertexArrays(1, &(figuras[i]).VAO_line);
    glDeleteBuffers(1, &(figuras[i]).VBO_line);
    glDeleteProgram(figuras[i].shaderProgram);
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}