#version 440 

// Source: http://blog.simonrodriguez.fr/articles/2016/07/implementing_fxaa.html

in vec2 uv;

out vec4 out_Color;

uniform sampler2D color;

const float EDGE_THRESHOLD_MIN = 0.0312;
const float EDGE_THRESHOLD_MAX = 0.125;
const float SUBPIXEL_QUALITY = 0.75;
const int ITERATIONS = 7;
const float QUALITY[7] = {1.5,2.0,2.0,2.0,2.0,4.0,8.0};

float rgb2luma(vec3 rgb) {
	return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}

void main(void) {
    vec2 texelStep = 1.0 / textureSize(color, 0);
    vec3 colorCenter = texture(color, uv).rgb;
	
	float lumaCenter = rgb2luma(colorCenter);
	
	float lumaDown = rgb2luma(textureOffset(color, uv, ivec2(0,-1)).rgb);
	float lumaUp = rgb2luma(textureOffset(color, uv, ivec2(0,1)).rgb);
	float lumaLeft = rgb2luma(textureOffset(color, uv, ivec2(-1,0)).rgb);
	float lumaRight = rgb2luma(textureOffset(color, uv, ivec2(1,0)).rgb);
	
	float lumaMin = min(lumaCenter,min(min(lumaDown,lumaUp),min(lumaLeft,lumaRight)));
	float lumaMax = max(lumaCenter,max(max(lumaDown,lumaUp),max(lumaLeft,lumaRight)));

	float lumaRange = lumaMax - lumaMin;

	if (lumaRange < max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX)) {
		out_Color = vec4(colorCenter, 1.0);
		return;
	}

	float lumaDownLeft = rgb2luma(textureOffset(color, uv, ivec2(-1,-1)).rgb);
	float lumaUpRight = rgb2luma(textureOffset(color, uv, ivec2(1,1)).rgb);
	float lumaUpLeft = rgb2luma(textureOffset(color, uv, ivec2(-1,1)).rgb);
	float lumaDownRight = rgb2luma(textureOffset(color, uv, ivec2(1,-1)).rgb);
	
	float lumaDownUp = lumaDown + lumaUp;
	float lumaLeftRight = lumaLeft + lumaRight;
	
	float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
	float lumaDownCorners = lumaDownLeft + lumaDownRight;
	float lumaRightCorners = lumaDownRight + lumaUpRight;
	float lumaUpCorners = lumaUpLeft + lumaUpRight;
	
	float edgeHorizontal = abs(-2.0 * lumaLeft + lumaLeftCorners) + abs(-2.0 * lumaCenter + lumaDownUp) * 2.0 + abs(-2.0 * lumaRight + lumaRightCorners);
	float edgeVertical = abs(-2.0 * lumaUp + lumaUpCorners) + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0 + abs(-2.0 * lumaDown + lumaDownCorners);

	bool isHorizontal = (edgeHorizontal >= edgeVertical);
	
	float luma1 = isHorizontal ? lumaDown : lumaLeft;
	float luma2 = isHorizontal ? lumaUp : lumaRight;
	
	float gradient1 = luma1 - lumaCenter;
	float gradient2 = luma2 - lumaCenter;
	
	bool is1Steepest = abs(gradient1) >= abs(gradient2);
	
	float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));
	
	float stepLength = isHorizontal ? texelStep.y : texelStep.x;
	
	float lumaLocalAverage = 0.0;
	
	if (is1Steepest) {
		stepLength = -stepLength;
		lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
	} else {
		lumaLocalAverage = 0.5 * (luma2 + lumaCenter);
	}
	
	vec2 currentUV = uv;
	if (isHorizontal) {
		currentUV.y += stepLength * 0.5;
	} else {
		currentUV.x += stepLength * 0.5;
	}
	
	vec2 offset = isHorizontal ? vec2(texelStep.x, 0.0)  : vec2(0.0, texelStep.y);
	vec2 uv1 = currentUV - offset;
	vec2 uv2 = currentUV + offset;
	
	float lumaEnd1 = rgb2luma(texture(color,uv1).rgb);
	float lumaEnd2 = rgb2luma(texture(color,uv2).rgb);
	lumaEnd1 -= lumaLocalAverage;
	lumaEnd2 -= lumaLocalAverage;
	
	bool reached1 = abs(lumaEnd1) >= gradientScaled;
	bool reached2 = abs(lumaEnd2) >= gradientScaled;
	bool reachedBoth = reached1 && reached2;
	
	if (!reached1) {
		uv1 -= offset;
	}
	if (!reached2) {
		uv2 += offset;
	}
	
	if (!reachedBoth) {
		for (int i = 2; i < ITERATIONS; i++) {
			if (!reached1) {
				lumaEnd1 = rgb2luma(texture(color, uv1).rgb);
				lumaEnd1 = lumaEnd1 - lumaLocalAverage;
			}
			if (!reached2) {
				lumaEnd2 = rgb2luma(texture(color, uv2).rgb);
				lumaEnd2 = lumaEnd2 - lumaLocalAverage;
			}
			reached1 = abs(lumaEnd1) >= gradientScaled;
			reached2 = abs(lumaEnd2) >= gradientScaled;
			reachedBoth = reached1 && reached2;
			if (!reached1) {
				uv1 -= offset * QUALITY[i];
			}
			if (!reached2) {
				uv2 += offset * QUALITY[i];
			}
			if (reachedBoth) break;
		}
	}
	
	float distance1 = isHorizontal ? (uv.x - uv1.x) : (uv.y - uv1.y);
	float distance2 = isHorizontal ? (uv2.x - uv.x) : (uv2.y - uv.y);
	
	bool isDirection1 = distance1 < distance2;
	float distanceFinal = min(distance1, distance2);
	
	float edgeThickness = (distance1 + distance2);
	
	float pixelOffset = - distanceFinal / edgeThickness + 0.5;
	
	bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
	
	bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;
	
	float finalOffset = correctVariation ? pixelOffset : 0.0;
	
	float lumaAverage = (1.0/12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
	
	float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter)/lumaRange, 0.0, 1.0);
	float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
	float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;
	
	finalOffset = max(finalOffset, subPixelOffsetFinal);
	
	vec2 finalUV = uv;
	if (isHorizontal) {
		finalUV.y += finalOffset * stepLength;
	} else {
		finalUV.x += finalOffset * stepLength;
	}
	
	vec3 finalColor = texture(color, finalUV).rgb;
	out_Color = vec4(finalColor, 1.0);
}