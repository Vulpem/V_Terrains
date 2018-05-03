#version 430 core

float heightScale = 1.f;
float triSize = 25.f;
//vec4 frustumPlanes[6];
vec3 tileSize = vec3(1.f,1.f,1.f);
//float tileBoundingSphereR = length(tileSize/2.f);
vec2 viewport = vec2(1920, 1080);


layout(vertices = 4) out;

uniform  mat4 view_matrix;
uniform  mat4 projection_matrix;

in  vec2 UV[];
in vec3 pos[];

out  vec2 UVs[];

//https://developer.nvidia.com/gameworks-vulkan-and-opengl-samples

// test if sphere is entirely contained within frustum planes
/*bool sphereInFrustum(vec3 pos, float r, vec4 plane[6])
{
    for (int i = 0; i<6; i++) {
        if (dot(vec4(pos, 1.0), plane[i]) + r < 0.0) {
            // sphere outside plane
            return false;
        }
    }
    return true;
}*/

// transform from world to screen coordinates
vec2 worldToScreen(vec3 p)
{
    vec4 r = (projection_matrix * view_matrix) * vec4(p, 1.0);   // to clip space
    r.xy /= r.w;            // project
    r.xy = r.xy*0.5 + 0.5;  // to NDC
    r.xy *= viewport.xy;    // to pixels
    return r.xy;
}

// calculate edge tessellation level from two edge vertices in screen space
float calcEdgeTessellation(vec2 s0, vec2 s1)
{
    float d = distance(s0, s1);
    return clamp(d / triSize, 1, 50);
}

vec2 eyeToScreen(vec4 p)
{
    vec4 r = projection_matrix * p;   // to clip space
    r.xy /= r.w;            // project
    r.xy = r.xy*0.5 + 0.5;  // to NDC
    r.xy *= viewport.xy;    // to pixels
    return r.xy;
}

// calculate tessellation level by fitting sphere to edge
float calcEdgeTessellationSphere(vec3 w0, vec3 w1, float diameter)
{
    vec3 centre = (w0 + w1) * 0.5;
    vec4 view0 = (view_matrix) * vec4(centre, 1.0);
    vec4 view1 = view0 + vec4(diameter, 0, 0, 0);
    vec2 s0 = eyeToScreen(view0);
    vec2 s1 = eyeToScreen(view1);
    return calcEdgeTessellation(s0, s1);
}

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    UVs[gl_InvocationID] = UV[gl_InvocationID];


    const float minX = min(min(pos[0].x, pos[1].x), min(pos[2].x, pos[3].x));
    const float minY = min(min(pos[0].y, pos[1].y), min(pos[2].y, pos[3].y));
    const float minZ = min(min(pos[0].z, pos[1].z), min(pos[2].z, pos[3].z));

    const float maxX = max(max(pos[0].x, pos[1].x), max(pos[2].x, pos[3].x));
    const float maxY = max(max(pos[0].y, pos[1].y), max(pos[2].y, pos[3].y));
    const float maxZ = max(max(pos[0].z, pos[1].z), max(pos[2].z, pos[3].z));

    // bounding sphere for patch
    //vec3 spherePos = vec3(minX + maxX, minY + maxY, minZ + maxZ) *0.5f;

    // test if patch is visible
    bool visible = true; //sphereInFrustum(spherePos, tileBoundingSphereR, frustumPlanes);

    if (!visible) {
        // cull patch
        gl_TessLevelOuter[0] = -1;
        gl_TessLevelOuter[1] = -1;
        gl_TessLevelOuter[2] = -1;
        gl_TessLevelOuter[3] = -1;

        gl_TessLevelInner[0] = -1;
        gl_TessLevelInner[1] = -1;
    }
    else {
            // compute automatic LOD

            // calculate edge tessellation levels
            // see tessellation diagram in OpenGL 4 specification for vertex order details

            // use screen space size of sphere fit to each edge
        float sphereD = max(maxX - minX, maxY - minY);// *2.0f;
            gl_TessLevelOuter[0] = calcEdgeTessellationSphere(pos[3], pos[0], sphereD);
            gl_TessLevelOuter[1] = calcEdgeTessellationSphere(pos[0], pos[1], sphereD);
            gl_TessLevelOuter[2] = calcEdgeTessellationSphere(pos[1], pos[2], sphereD);
            gl_TessLevelOuter[3] = calcEdgeTessellationSphere(pos[2], pos[3], sphereD);

            // calc interior tessellation level - use average of outer levels
            gl_TessLevelInner[0] = 0.5 * (gl_TessLevelOuter[1] + gl_TessLevelOuter[3]);
            gl_TessLevelInner[1] = 0.5 * (gl_TessLevelOuter[0] + gl_TessLevelOuter[2]);

            //Out[gl_InvocationID].tessLevelInner = vec2(gl_TessLevelInner[0], gl_TessLevelInner[1]);
    }
}