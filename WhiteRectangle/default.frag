#version 330 core
out vec4 FragColor;

uniform mat4 u_transform;
uniform vec2 u_res;
uniform float u_time;
uniform float u_noiseProb;
uniform int u_seed;
uniform vec2 u_a;
uniform vec2 u_b;

float rand4(float key1, vec2 key2)
{
    return fract(sin(float(u_seed) / 100000.f + key1 + dot(key2, vec2(12.9898,78.233)))*43758.5453123);
}

void main()
{
    vec4 p = u_transform * gl_FragCoord;
    vec2 st = p.xy/u_res.xy;
    float r = rand4(u_time, st);
	//x = ax * t + bx, y = ay * t + by
	vec2 movingPoint = u_a * u_time + u_b; //0.1 0.1

    if(r <= u_noiseProb)
    {
        FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        FragColor = vec4(1.f, 1.f, 1.f, 1.0f);
    }
	
    float x_tar = movingPoint.x;
    float y_tar = movingPoint.y;

    if(p.x+1 >= x_tar && p.x-1 <= x_tar)
        if(p.y+1 >= y_tar && p.y-1 <= y_tar)
        {
            FragColor.rgb = vec3(1.0f, 0.0f, 0.0f);
        }
} 
