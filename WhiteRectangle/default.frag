#version 330 core
out vec4 FragColor;

uniform mat4 u_transform;
uniform vec2 u_res;
uniform float u_time;
uniform float u_eps;
uniform float u_noiseProb;

float rand(float key1, vec2 key2)
{
    return fract(sin(key1*1000000.f * length(key2)));
}

float rand2(float key1, vec2 key2)
{
    return fract(sin(key1*length(key2))*100000.0f);
}

float rand3(float key1, vec2 key2)
{
    return fract(sin((key2.x*key2.x+key2.y*key2.y)*10000.f));
}

float rand4(float key1, vec2 key2)
{
    return fract(sin(key1+dot(key2, vec2(12.9898,78.233)))*43758.5453123);
}

void main()
{
    vec4 p = u_transform * gl_FragCoord;
    vec2 st = p.xy/u_res.xy;
    float r = rand4(u_time, st);

    vec2 movingPoint = vec2(0.0f + 0.1f * u_time, 0.0f + 0.1f * u_time);
    vec2 mask = st.xy / movingPoint.xy;

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
    if(st.x >= x_tar-u_eps && st.x <= x_tar+u_eps)
        if(st.y >= y_tar-u_eps && st.y <= y_tar+u_eps)
        {
            FragColor.rgb = vec3(1.0f, 0.0f, 0.0f);
        }
    //FragColor.xy = movingPoint.xy
    //FragColor = vec4(vec3(r), 1.0f);
} 