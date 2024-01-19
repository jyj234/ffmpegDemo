varying vec2 v_texCoord;

uniform sampler2D tex_y;
uniform sampler2D tex_u;
uniform sampler2D tex_v;
uniform sampler2D tex_uv;
uniform int pixFmt;
void main(void)
{
    vec3 yuv;
    vec3 rgb;
    if (pixFmt == 0) {
        //yuv420p
        yuv.x = texture2D(tex_y, v_texCoord).r;
        yuv.y = texture2D(tex_u, v_texCoord).r - 0.5;
        yuv.z = texture2D(tex_v, v_texCoord).r - 0.5;
        rgb = mat3( 1.0,       1.0,         1.0,
                        0.0,       -0.3455,  1.779,
                        1.4075, -0.7169,  0.0) * yuv;
    }
    else if(pixFmt == 1){
        //NV12
        yuv.x = texture2D(tex_y, v_texCoord).r;
        yuv.y = texture2D(tex_uv, v_texCoord).r - 0.5;
        yuv.z = texture2D(tex_uv, v_texCoord).g - 0.5;
        rgb = mat3( 1.0,       1.0,         1.0,
                        0.0,       -0.3455,  1.779,
                        1.4075, -0.7169,  0.0) * yuv;
    }
    gl_FragColor = vec4(rgb, 1.0);
}
