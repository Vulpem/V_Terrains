#version 430 core

float heightScale = 1.f;
float triSize = 30.f;
//vec4 frustumPlanes[6];
vec3 tileSize = vec3(1.f, 1.f, 1.f);
//float tileBoundingSphereR = length(tileSize/2.f);
vec2 viewport = vec2(1920, 1080);


layout(vertices = 4) out;

uniform  mat4 view_matrix;
uniform  mat4 projection_matrix;

in  vec2 UV[];
in vec3 pos[];

out  vec2 UVs[];

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
            p.x > 0.0f - allowance && p.x < 1.f + allowance
         && p.y > 0.0f - allowance && p.y < 1.f + allowance
         && p.z > 0.0f);
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

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    UVs[gl_InvocationID] = UV[gl_InvocationID];

    vec3 spherePos = (pos[0] + pos[1] + pos[2] + pos[3]) *0.25f;
    float sphereDiam = distance(spherePos, pos[0]) * 1.1f;
    vec2 diamOnScreen = eyeToScreen(vec4(sphereDiam, 0.f, 0.f, 1.f)).xy;

    if (!sphereInScreen(spherePos, -0.1))//length(diamOnScreen) / 2.f))
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
        gl_TessLevelOuter[0] = calcEdgeTessellationSphere(pos[3], pos[0], sphereDiam);
        gl_TessLevelOuter[1] = calcEdgeTessellationSphere(pos[0], pos[1], sphereDiam);
        gl_TessLevelOuter[2] = calcEdgeTessellationSphere(pos[1], pos[2], sphereDiam);
        gl_TessLevelOuter[3] = calcEdgeTessellationSphere(pos[2], pos[3], sphereDiam);

        gl_TessLevelInner[0] = 0.5 * (gl_TessLevelOuter[1] + gl_TessLevelOuter[3]);
        gl_TessLevelInner[1] = 0.5 * (gl_TessLevelOuter[0] + gl_TessLevelOuter[2]);
    }
}