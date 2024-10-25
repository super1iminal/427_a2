#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float darken_screen_factor;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec2 distort(vec2 uv) 
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE THE WATER DISTORTION HERE (you may want to try sin/cos)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	float x = uv.x;
    float y = uv.y;
	y = y + 0.004 * sin(10.0 * x + time/5.0);
	while (y >= 1.0) y -= 1.0;
    while (y <= 0.0) y += 1.0;
	x = x + 0.004 * sin(10.0 * y + time/5.0);
	while (x >= 1.0) x -= 1.0;
	while (x <= 0.0) x += 1.0;


	return vec2(x, y);
}

vec4 color_shift(vec4 in_color) 
{
    float speed = 0.1; 
    
    float shift_factor = sin(time * speed) * 0.2 + 0.2; // Normalize to range [0, 1]
    
    vec4 blueish_yellow = vec4(0.2, 0.8, 1.0, 1.0); // A blueish-yellow tint
    
    in_color = mix(in_color, blueish_yellow, shift_factor);
    
    return in_color;
}





vec4 fade_color(vec4 in_color) 
{
	if (darken_screen_factor > 0)
		in_color -= darken_screen_factor * vec4(0.8, 0.8, 0.8, 0);
	return in_color;
}

void main()
{
	vec2 coord = distort(texcoord);

    vec4 in_color = texture(screen_texture, coord);
    color = color_shift(in_color);
    color = fade_color(color);
}