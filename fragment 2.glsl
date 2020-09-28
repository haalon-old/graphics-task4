#version 330 core
in vec3 vFragPosition;
in vec2 vTexCoords;
in vec3 vNormal;

out vec4 color;

uniform sampler2D water_texture;
uniform sampler2D sand_texture;
uniform sampler2D grass_texture;
uniform sampler2D stone_texture;

uniform bool state;

void main()
{
  vec3 lightDir = vec3(1.0f, 1.0f, 0.0f); 
  vec3 col;
  float height = vFragPosition.y;

  if(state)
  {
  	color = vec4( vec3(0.5f,0.5f,0.5f)+vNormal/2,1.0f);
  }
  else
  {
	  if(height<-0.2)
	  {
	  	col = texture(water_texture, vTexCoords).xyz;
	  } else if(height<0)
	  {
	  	col = mix(texture(water_texture, vTexCoords), texture(sand_texture, 4*vTexCoords), (height+0.2)*5).xyz;
	  }
	   else if(height<0.5)
	  {
	  	col = texture(sand_texture, 4*vTexCoords).xyz;
	  } else if(height<1)
	  {
	  	col = mix(texture(sand_texture,4* vTexCoords), texture(grass_texture, 4*vTexCoords), (height-0.5)*2).xyz;
	  }
	  else if(height<5)
	  {
	  	col = texture(grass_texture, 4*vTexCoords).xyz;
	  }else if (height<6)
	  {
	  	col = mix(texture(grass_texture, 4*vTexCoords), texture(stone_texture, 2*vTexCoords), (height-5)).xyz;
	  }
	  else col = texture(stone_texture, 2*vTexCoords).xyz;
	   

	  float kd = max(dot(vNormal, lightDir), 0.0);

	  color = vec4(kd * col, 1.0f);
	}
}


