#version 410 core

//TODO send as uniforms
float triSize = 5.f;
vec2 viewport = vec2(1920, 1080);


layout(vertices = 4) out;

uniform mat4 model_matrix;
uniform  mat4 view_matrix;
uniform  mat4 projection_matrix;

in vec3 pos[];
in vec2 UV[];

out vec3 position[];
out vec2 UVs[];

//https://developer.nvidia.com/gameworks-vulkan-and-opengl-samples

vec4 worldToScreen(vec3 p)
{
    vec4 r = (projection_matrix * view_matrix) * vec4(p, 1.0);   // to clip space
    r.xyz /= r.w;            // project
    r.xyz = r.xyz*0.5 + 0.5;  // to NDC
    return r;
}

vec4 eyeToScreen(vec4 p)
{
    vec4 r = projection_matrix * p;   // to clip space
    r.xyz /= r.w;            // project
    r.xyz = r.xyz*0.5 + 0.5;  // to NDC
    return r;
}

bool sphereInScreen(vec3 pos, float allowance)
{
    vec4 p = worldToScreen(pos);
    return (
            (p.x > 0.0f - allowance && p.x < 1.f + allowance)
        // && p.y > 0.0f - allowance && p.y < 1.f + allowance
         || p.z < 0.995);
}

float calcEdgeTessellation(vec2 s0, vec2 s1)
{
    float d = distance(s0 * viewport, s1 * viewport);
    return clamp(d / triSize, 1, 61);
}

float calcEdgeTessellationSphere(vec3 w0, vec3 w1, float diameter)
{
    vec3 centre = (w0 + w1) * 0.5;
    vec4 view0 = (view_matrix)* vec4(centre, 1.0);
    vec4 view1 = view0 + vec4(diameter, 0, 0, 0);
    vec2 s0 = eyeToScreen(view0).xy;
    vec2 s1 = eyeToScreen(view1).xy;
    return calcEdgeTessellation(s0, s1);
}

int calcPowEdgeTessellationSphere(vec3 w0, vec3 w1, float diameter)
{
   int tess = int(floor(calcEdgeTessellationSphere(w0, w1, diameter)));
   int n = 1;
   while(tess > int(pow(2,n)))
   {

       n++;
   }

   return int(pow(2,n-1));
}

void main()
{
    position[gl_InvocationID] = pos[gl_InvocationID];
    UVs[gl_InvocationID] = UV[gl_InvocationID];

	vec3 p[4];
	p[0] = (model_matrix * vec4(pos[0], 1.f)).xyz;
	p[1] = (model_matrix * vec4(pos[1], 1.f)).xyz;
	p[2] = (model_matrix * vec4(pos[2], 1.f)).xyz;
	p[3] = (model_matrix * vec4(pos[3], 1.f)).xyz;

    vec3 spherePos = (p[0] + p[1] + p[2] + p[3]) *0.25f;
    float sphereDiam = distance(spherePos, p[0]) * 1.1f;
    vec2 diamOnScreen = eyeToScreen(vec4(sphereDiam, 0.f, 0.f, 1.f)).xy;

    if (!sphereInScreen(spherePos, 0.15))//length(diamOnScreen) / 2.f))
    {
        gl_TessLevelOuter[0] = -1;
        gl_TessLevelOuter[1] = -1;
        gl_TessLevelOuter[2] = -1;
        gl_TessLevelOuter[3] = -1;

        gl_TessLevelInner[0] = -1;
        gl_TessLevelInner[1] = -1;
    }
    else
    {
        gl_TessLevelOuter[0] = calcPowEdgeTessellationSphere(p[3], p[0], sphereDiam);
        gl_TessLevelOuter[1] = calcPowEdgeTessellationSphere(p[0], p[1], sphereDiam);
        gl_TessLevelOuter[2] = calcPowEdgeTessellationSphere(p[1], p[2], sphereDiam);
        gl_TessLevelOuter[3] = calcPowEdgeTessellationSphere(p[2], p[3], sphereDiam);

        gl_TessLevelInner[1] = gl_TessLevelInner[0] = gl_TessLevelOuter[0];
    }
}







