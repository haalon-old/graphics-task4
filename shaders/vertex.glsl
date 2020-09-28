#version 330 core
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;


out vec2 vTexCoords;
out vec3 vFragPosition;
out vec3 vNormal;
flat out int water;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;


uniform float dx;
uniform int n_warp;

void main()
{
	vec3 vert = vertex;
	water=0;
	vert.x+=((gl_InstanceID% (n_warp*n_warp))/n_warp-n_warp/2)*dx;
	vert.z+=((gl_InstanceID % (n_warp*n_warp))%n_warp-n_warp/2)*dx;
  if(gl_InstanceID>=n_warp*n_warp)
  {
  	vert.y = (cos(time*2+cos(vert.x+vert.z)*16+cos(vert.z-vert.x)*16)-1)/10 + (cos(time/5)-1)/4;
  	water=1;	
  }

  gl_Position = projection * view * model * vec4(vert, 1.0f);
 	

  vTexCoords = texCoords;
  vFragPosition = vec3(model * vec4(vert, 1.0f));
  vNormal = mat3(transpose(inverse(model))) * normal;
}

