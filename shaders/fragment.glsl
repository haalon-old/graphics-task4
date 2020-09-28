#version 330 core
in vec3 vFragPosition;
in vec2 vTexCoords;
in vec3 vNormal;
flat in int water;


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
  vec4 fog = vec4(1.0f, 1.0f, 1.0f,1.0f);
  float height = vFragPosition.y;
  float fog_coord =  exp(-pow(1.0 / (0.01 * abs(gl_FragCoord.z / gl_FragCoord.w)), 2.0)) ;
  //exp(-pow(1.0 / (fog_density * fog_coord), 2.0))
  if(state)
  {	
  	color = vec4( vec3(0.5f,0.5f,0.5f)+vNormal/2,1.0f);
  }
  else if(water==1)
  {
		color = texture(water_texture, vTexCoords);
		color.w = 0.5f;
		color = mix(color, fog,fog_coord );
  }
  else
  {
	  
	  
	  if(height<0.5)
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
		color = mix(color, fog,fog_coord );
	}
}
