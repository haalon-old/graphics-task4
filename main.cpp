//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "Camera.h"

//External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include "SOIL/SOIL.h"
#include <random>

#include "matrix.h"

using std::cout;

static const GLsizei WIDTH = 1920, HEIGHT = 1080; //размеры окна
static int filling = 0;
static bool keys[1024]; //массив состояний кнопок - нажата/не нажата
static GLfloat lastX = 400, lastY = 300; //исходное положение мыши
static bool firstMouse = true;
static bool g_captureMouse         = true;  // Мышка захвачена нашим приложением или нет?
static bool g_capturedMouseJustNow = false;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

Camera camera(float3(0.0f, 15.0f, 0.0f));

bool normal_flag = false;
bool loop_flag = false;

//функция для обработки нажатий на кнопки клавиатуры
void OnKeyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//std::cout << key << std::endl;
	switch (key)
	{
	case GLFW_KEY_ESCAPE: //на Esc выходим из программы
		if (action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		break;
	case GLFW_KEY_SPACE: //на пробел переключение в каркасный режим и обратно
		if (action == GLFW_PRESS)
		{
			if (filling == 0)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				filling = 1;
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				filling = 0;
			}
		}
		break;
  case GLFW_KEY_1:
    normal_flag = false;
    break;
  case GLFW_KEY_2:
    normal_flag = true;
    break;
	default:
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

//функция для обработки клавиш мыши
void OnMouseButtonClicked(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    g_captureMouse = !g_captureMouse;


  if (g_captureMouse)
  {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    g_capturedMouseJustNow = true;
  }
  else
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

}

//функция для обработки перемещения мыши
void OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
  if (firstMouse)
  {
    lastX = float(xpos);
    lastY = float(ypos);
    firstMouse = false;
  }

  GLfloat xoffset = float(xpos) - lastX;
  GLfloat yoffset = lastY - float(ypos);  

  lastX = float(xpos);
  lastY = float(ypos);

  if (g_captureMouse)
    camera.ProcessMouseMove(xoffset, yoffset);
}


void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
  camera.ProcessMouseScroll(GLfloat(yoffset));
}

void doCameraMovement(Camera &camera, GLfloat deltaTime)
{
  if (keys[GLFW_KEY_W])
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (keys[GLFW_KEY_A])
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (keys[GLFW_KEY_S])
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (keys[GLFW_KEY_D])
    camera.ProcessKeyboard(RIGHT, deltaTime);
}

void warpCamera(Camera &camera, float bound)
{
  camera.pos.x =  camera.pos.x < - bound ? 2*bound+camera.pos.x: (camera.pos.x > bound ? -2*bound+camera.pos.x : camera.pos.x );
  //camera.pos.y =  camera.pos.y < - bound ? 2*bound+camera.pos.y: fmod(camera.pos.y,bound);
  camera.pos.z =  camera.pos.z < - bound ? 2*bound+camera.pos.z: (camera.pos.z > bound ? -2*bound+camera.pos.z : camera.pos.z );
}


std::random_device rd;  
std::mt19937 gen(rd()); 
std::uniform_real_distribution<> dis(0.0, 1.0);
float rnd()
{     
  return dis(gen);   
}

float shape = 32;


void diamond(Matrix<float> & heights, int pos_x, int pos_z, int delta)
{

  if(pos_x == heights.n_cols-1 && loop_flag)
  {
    heights(pos_x,pos_z) = heights(0,pos_z);
    return;
  }

  if(pos_z == heights.n_cols-1 && loop_flag)
  {
    heights(pos_x,pos_z) = heights(pos_x,0);
    return;
  }

  float height = 0, delim =4;
  int x,z;
  for (int i = 0; i < 4; ++i)
  {

    x = i%2 ? pos_x  : pos_x + (i/2 ? delta :-delta); 
    z = !(i%2) ? pos_z  : pos_z + (i/2 ? delta :-delta);
    if(loop_flag)
    {
      x = x<0? heights.n_cols +x-1: ( x>=heights.n_cols? x%heights.n_cols +1:x);
      z = z<0? heights.n_cols +z-1: ( z>=heights.n_cols? z%heights.n_cols +1:z);
     
      height+=heights(x,z);
    }
    else
    {
      if(x>=0 && x < heights.n_cols && z>=0 && z < heights.n_cols)
        height+=heights(x,z);
      else
        delim=3;
    }    
    
  }
  height/=delim;
  height+= (1-2*rnd())*delta/(heights.n_cols-1)*shape;
  heights(pos_x,pos_z) = height;
}

void diamonds(Matrix<float> & heights, uint a_x, uint a_z, uint b_x, uint b_z)
{
  uint center_x = (a_x+b_x)/2;
  uint center_z = (a_z+b_z)/2;
  uint delta = center_x-a_x;

  diamond(heights, a_x, center_z, delta);
  diamond(heights, center_x, a_z, delta);  

  diamond(heights, b_x, center_z, delta);
  diamond(heights, center_x, b_z, delta);
}

void square(Matrix<float> & heights, uint a_x, uint a_z, uint b_x, uint b_z)
{
  uint center_x = (a_x+b_x)/2;
  uint center_z = (a_z+b_z)/2;

  heights(center_x,center_z) = (heights(a_x,a_z)+heights(a_x,b_z)+heights(b_x,b_z)+heights(b_x,a_z))/4 +(1-2*rnd())*(center_x-a_x)/(heights.n_cols-1)*shape;
}

void init(Matrix<float> & heights)
{
  if(loop_flag)
  {
    heights(0,0)=0;
    heights(heights.n_cols-1,0)=0;
    heights(0,heights.n_cols-1)=0;
    heights(heights.n_cols-1,heights.n_cols-1)=0;
  }
  else
  {
    heights(0,0)=(1-2*rnd())*shape;
    heights(heights.n_cols-1,0)=(1-2*rnd())*shape;
    heights(0,heights.n_cols-1)=(1-2*rnd())*shape;
    heights(heights.n_cols-1,heights.n_cols-1)=-heights(0,0);
  }
  
}

void create_heightmap(Matrix<float> & heights)
{
  uint pow = 1,count = 1, square_size = heights.n_cols-1, shift,i,j;
  uint n_cols = heights.n_cols;
  init(heights);
  while(square_size>1)
  {
    //cout<<"starting iteration with size: "<<square_size<<" and count: "<<count<<"\n";
    for(uint k = 0;k<count;k++)
    {
      i=k/pow;j=k%pow;
      square(heights,i*square_size, j*square_size, (i+1)*square_size, (j+1)*square_size);
    }
    //cout<<"square succ\n";
    for(uint k = 0;k<count;k++)
    {
      i=k/pow;j=k%pow;
      diamonds(heights,i*square_size, j*square_size, (i+1)*square_size, (j+1)*square_size);
    }
    //cout<<"diamonds succ\n";
    count*=4;
    pow*=2;
    square_size/=2;
  }
   

}

/**
\brief создать triangle strip плоскость и загрузить её в шейдерную программу
\param rows - число строк
\param cols - число столбцов
\param size - размер плоскости
\param vao - vertex array object, связанный с созданной плоскостью
*/
static int createTriStrip(int rows, int cols, float size, GLuint &vao)
{

  int numIndices = 2 * cols*(rows - 1) + rows - 1;

  std::vector<GLfloat> vertices_vec; //вектор атрибута координат вершин
  vertices_vec.reserve(rows * cols * 3);

  std::vector<GLfloat> normals_vec; //вектор атрибута нормалей к вершинам
  normals_vec.reserve(rows * cols * 3);

  std::vector<GLfloat> texcoords_vec; //вектор атрибут текстурных координат вершин
  texcoords_vec.reserve(rows * cols * 2);

  std::vector<float3> normals_vec_tmp(rows * cols, float3(0.0f, 0.0f, 0.0f)); //временный вектор нормалей, используемый для расчетов

  std::vector<int3> faces;         //вектор граней (треугольников), каждая грань - три индекса вершин, её составляющих; используется для удобства расчета нормалей
  faces.reserve(numIndices / 3);

  std::vector<GLuint> indices_vec; //вектор индексов вершин для передачи шейдерной программе
  indices_vec.reserve(numIndices);

  Matrix<float> y_values(rows,cols);
  

  create_heightmap(y_values);



  for (int z = 0; z < rows; ++z)
  {
    for (int x = 0; x < cols; ++x)
    {
      //вычисляем координаты каждой из вершин 
      float xx = -size / 2 + x*size / cols;
      float zz = -size / 2 + z*size / rows;
      // float yy = -1.0f;
      float r = sqrt(xx*xx + zz*zz);
      float yy = y_values(z,x);
      //yy*=abs(yy)/shape;
     
      vertices_vec.push_back(xx);
      vertices_vec.push_back(yy);
      vertices_vec.push_back(zz);

      texcoords_vec.push_back(x / float(cols - 1)); // вычисляем первую текстурную координату u, для плоскости это просто относительное положение вершины
      texcoords_vec.push_back(z / float(rows - 1)); // аналогично вычисляем вторую текстурную координату v
    }
  }

  //primitive restart - специальный индекс, который обозначает конец строки из треугольников в triangle_strip
  //после этого индекса формирование треугольников из массива индексов начнется заново - будут взяты следующие 3 индекса для первого треугольника
  //и далее каждый последующий индекс будет добавлять один новый треугольник пока снова не встретится primitive restart index

  int primRestart = cols * rows;

  for (int x = 0; x < cols - 1; ++x)
  {
    for (int z = 0; z < rows - 1; ++z)
    {
      int offset = x*cols + z;

      //каждую итерацию добавляем по два треугольника, которые вместе формируют четырехугольник
      if (z == 0) //если мы в начале строки треугольников, нам нужны первые четыре индекса
      {
        indices_vec.push_back(offset + 0);
        indices_vec.push_back(offset + rows);
        indices_vec.push_back(offset + 1);
        indices_vec.push_back(offset + rows + 1);
      }
      else // иначе нам достаточно двух индексов, чтобы добавить два треугольника
      {
        indices_vec.push_back(offset + 1);
        indices_vec.push_back(offset + rows + 1);

        if (z == rows - 2) indices_vec.push_back(primRestart); // если мы дошли до конца строки, вставляем primRestart, чтобы обозначить переход на следующую строку
      }
    }
  }

  ///////////////////////
  //формируем вектор граней(треугольников) по 3 индекса на каждый
  int currFace = 1;
  for (int i = 0; i < indices_vec.size() - 2; ++i)
  {
    int3 face;

    int index0 = indices_vec.at(i);
    int index1 = indices_vec.at(i + 1);
    int index2 = indices_vec.at(i + 2);

    if (index0 != primRestart && index1 != primRestart && index2 != primRestart)
    {
      if (currFace % 2 != 0) //если это нечетный треугольник, то индексы и так в правильном порядке обхода - против часовой стрелки
      {
        face.x = indices_vec.at(i);
        face.y = indices_vec.at(i + 1);
        face.z = indices_vec.at(i + 2);

        currFace++;
      }
      else //если треугольник четный, то нужно поменять местами 2-й и 3-й индекс;
      {    //при отрисовке opengl делает это за нас, но при расчете нормалей нам нужно это сделать самостоятельно
        face.x = indices_vec.at(i);
        face.y = indices_vec.at(i + 2);
        face.z = indices_vec.at(i + 1);

        currFace++;
      }
      faces.push_back(face);
    }
  }


  ///////////////////////
  //расчет нормалей
  for (int i = 0; i < faces.size(); ++i)
  {
    //получаем из вектора вершин координаты каждой из вершин одного треугольника
    float3 A(vertices_vec.at(3 * faces.at(i).x + 0), vertices_vec.at(3 * faces.at(i).x + 1), vertices_vec.at(3 * faces.at(i).x + 2));
    float3 B(vertices_vec.at(3 * faces.at(i).y + 0), vertices_vec.at(3 * faces.at(i).y + 1), vertices_vec.at(3 * faces.at(i).y + 2));
    float3 C(vertices_vec.at(3 * faces.at(i).z + 0), vertices_vec.at(3 * faces.at(i).z + 1), vertices_vec.at(3 * faces.at(i).z + 2));

    //получаем векторы для ребер треугольника из каждой из 3-х вершин
    float3 edge1A(normalize(B - A));
    float3 edge2A(normalize(C - A));

    float3 edge1B(normalize(A - B));
    float3 edge2B(normalize(C - B));

    float3 edge1C(normalize(A - C));
    float3 edge2C(normalize(B - C));

    //нормаль к треугольнику - векторное произведение любой пары векторов из одной вершины
    float3 face_normal = cross(edge1A, edge2A);

    //простой подход: нормаль к вершине = средняя по треугольникам, к которым принадлежит вершина
    normals_vec_tmp.at(faces.at(i).x) += face_normal;
    normals_vec_tmp.at(faces.at(i).y) += face_normal;
    normals_vec_tmp.at(faces.at(i).z) += face_normal;
  }
  //float3 temp;
  for (int i = 0; i < cols; ++i)
  {
    normals_vec_tmp.at(i)+=normals_vec_tmp.at(i+cols*(cols-1));
    normals_vec_tmp.at(i+cols*(cols-1)) = normals_vec_tmp.at(i);
  }

  for (int i = 0; i < cols; ++i)
  {
    normals_vec_tmp.at(i*cols)+=normals_vec_tmp.at(i*cols+(cols-1));
    normals_vec_tmp.at(i*cols+(cols-1)) = normals_vec_tmp.at((i*cols));
  }
  //нормализуем векторы нормалей и записываем их в вектор из GLFloat, который будет передан в шейдерную программу
  for (int i = 0; i < normals_vec_tmp.size(); ++i)
  {
    float3 N = normalize(normals_vec_tmp.at(i));

    normals_vec.push_back(N.x);
    normals_vec.push_back(N.y);
    normals_vec.push_back(N.z);
  }


  GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vboVertices);
  glGenBuffers(1, &vboIndices);
  glGenBuffers(1, &vboNormals);
  glGenBuffers(1, &vboTexCoords);


  glBindVertexArray(vao); GL_CHECK_ERRORS;
  {

    //передаем в шейдерную программу атрибут координат вершин
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices); GL_CHECK_ERRORS;
    glBufferData(GL_ARRAY_BUFFER, vertices_vec.size() * sizeof(GL_FLOAT), &vertices_vec[0], GL_STATIC_DRAW); GL_CHECK_ERRORS;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0); GL_CHECK_ERRORS;
    glEnableVertexAttribArray(0); GL_CHECK_ERRORS;

    //передаем в шейдерную программу атрибут нормалей
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals); GL_CHECK_ERRORS;
    glBufferData(GL_ARRAY_BUFFER, normals_vec.size() * sizeof(GL_FLOAT), &normals_vec[0], GL_STATIC_DRAW); GL_CHECK_ERRORS;
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0); GL_CHECK_ERRORS;
    glEnableVertexAttribArray(1); GL_CHECK_ERRORS;

    //передаем в шейдерную программу атрибут текстурных координат
    glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords); GL_CHECK_ERRORS;
    glBufferData(GL_ARRAY_BUFFER, texcoords_vec.size() * sizeof(GL_FLOAT), &texcoords_vec[0], GL_STATIC_DRAW); GL_CHECK_ERRORS;
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), (GLvoid*)0); GL_CHECK_ERRORS;
    glEnableVertexAttribArray(2); GL_CHECK_ERRORS;

    //передаем в шейдерную программу индексы
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices); GL_CHECK_ERRORS;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_vec.size() * sizeof(GLuint), &indices_vec[0], GL_STATIC_DRAW); GL_CHECK_ERRORS;

    glEnable(GL_PRIMITIVE_RESTART); GL_CHECK_ERRORS;
    glPrimitiveRestartIndex(primRestart); GL_CHECK_ERRORS;
  }
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(0);


  return numIndices;
}


int initGL()
{
	int res = 0;

	//грузим функции opengl через glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	//выводим в консоль некоторую информацию о драйвере и контексте opengl
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  std::cout << "Controls: "<< std::endl;
  std::cout << "press left mose button to capture/release mouse cursor  "<< std::endl;
  std::cout << "press spacebar to alternate between shaded wireframe and fill display modes" << std::endl;
  std::cout << "press ESC to exit" << std::endl;

	return 0;
}

int main(int argc, char** argv)
{
	if(!glfwInit())
    return -1;
  float size = 160;
  int dimension = 129;
  loop_flag=argc>1;
	//запрашиваем контекст opengl версии 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); 


  GLFWwindow*  window = glfwCreateWindow(WIDTH, HEIGHT, "Landscape", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window); 

	//регистрируем коллбеки для обработки сообщений от пользователя - клавиатура, мышь..
	glfwSetKeyCallback        (window, OnKeyboardPressed);  
	glfwSetCursorPosCallback  (window, OnMouseMove); 
  glfwSetMouseButtonCallback(window, OnMouseButtonClicked);
	glfwSetScrollCallback     (window, OnMouseScroll);
	glfwSetInputMode          (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

	if(initGL() != 0) 
		return -1;
	
  //Reset any OpenGL errors which could be present for some reason
	GLenum gl_error = glGetError();
	while (gl_error != GL_NO_ERROR)
		gl_error = glGetError();

	//создание шейдерной программы из двух файлов с исходниками шейдеров
	//используется класс-обертка ShaderProgram
	std::unordered_map<GLenum, std::string> shaders;
	shaders[GL_VERTEX_SHADER]   = "vertex.glsl";
	shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
	ShaderProgram program(shaders); GL_CHECK_ERRORS;

  
  //Создаем и загружаем геометрию поверхности
  GLuint vaoTriStrip, vaoWater;
  int triStripIndices = createTriStrip(dimension, dimension, size, vaoTriStrip);
  
  
  


  glViewport(0, 0, WIDTH, HEIGHT);  GL_CHECK_ERRORS;
  glEnable(GL_DEPTH_TEST);  GL_CHECK_ERRORS;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



  GLuint water_texture;
  glGenTextures(1, &water_texture);
  glBindTexture(GL_TEXTURE_2D, water_texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width_img, height_img;
  unsigned char* image = SOIL_load_image("bsod.jpg", &width_img, &height_img, 0, SOIL_LOAD_RGB);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_img, height_img, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);
  SOIL_free_image_data(image);
  glBindTexture(GL_TEXTURE_2D, 0);


  GLuint sand_texture;
  glGenTextures(1, &sand_texture);
  glBindTexture(GL_TEXTURE_2D, sand_texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  image = SOIL_load_image("sand.jpg", &width_img, &height_img, 0, SOIL_LOAD_RGB);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_img, height_img, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);
  SOIL_free_image_data(image);
  glBindTexture(GL_TEXTURE_2D, 0);  


  GLuint grass_texture;
  glGenTextures(1, &grass_texture);
  glBindTexture(GL_TEXTURE_2D, grass_texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  image = SOIL_load_image("grass.jpg", &width_img, &height_img, 0, SOIL_LOAD_RGB);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_img, height_img, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);
  SOIL_free_image_data(image);
  glBindTexture(GL_TEXTURE_2D, 0);


  GLuint stone_texture;
  glGenTextures(1, &stone_texture);
  glBindTexture(GL_TEXTURE_2D, stone_texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  image = SOIL_load_image("stone.jpg", &width_img, &height_img, 0, SOIL_LOAD_RGB);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_img, height_img, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);
  SOIL_free_image_data(image);
  glBindTexture(GL_TEXTURE_2D, 0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	int n_warp = loop_flag ? 5 :1;

  program.StartUseShader(); GL_CHECK_ERRORS;
    
    
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, water_texture);
  program.SetUniform("water_texture", 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, sand_texture);
  program.SetUniform("sand_texture", 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, grass_texture);
  program.SetUniform("grass_texture", 2);

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, stone_texture);
  program.SetUniform("stone_texture", 3);
  program.SetUniform("dx",      (size-size/dimension));
  program.SetUniform("n_warp",      n_warp);
  float4x4 model; 
  program.SetUniform("model",      model);
  glClearColor(0.82f, 0.8f, 0.92f, 1.0f); GL_CHECK_ERRORS;
	while (!glfwWindowShouldClose(window))
	{
		
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

    if(loop_flag)
      warpCamera(camera, size/2-size/dimension/2);

		glfwPollEvents();
    doCameraMovement(camera, deltaTime);
    


		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS;

    

		
    float4x4 view       = camera.GetViewMatrix();
    float4x4 projection = projectionMatrixTransposed(camera.zoom, float(WIDTH) / float(HEIGHT), 0.1f, 1000.0f);

		                
		
		
    program.SetUniform("view",       view);       GL_CHECK_ERRORS;
    program.SetUniform("projection", projection); GL_CHECK_ERRORS;
    

    program.SetUniform("state",      normal_flag);
    
    GLfloat timeValue = glfwGetTime();
    //gl_InstanceID
    
     
    
    program.SetUniform("time",      timeValue);
    
      //program.SetUniform("water",      false);          
    //glDepthMask(false);
    glBindVertexArray(vaoTriStrip);
    glDrawElementsInstanced(GL_TRIANGLE_STRIP, triStripIndices, GL_UNSIGNED_INT, nullptr,2*n_warp*n_warp); GL_CHECK_ERRORS;
    glBindVertexArray(0); GL_CHECK_ERRORS;
    

    

		glfwSwapBuffers(window); 
	}
  program.StopUseShader();
	
	glDeleteVertexArrays(1, &vaoTriStrip);

	glfwTerminate();
	return 0;
}
