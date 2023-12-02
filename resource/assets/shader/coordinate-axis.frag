#version 330 core

const float subLineGrayVal = 0.55;

uniform float width;
uniform float spacing;
uniform float scale;
uniform vec2 screenSize;
uniform vec2 cameraPos;

bool modified(float a){
	return a > 0.0;
}

void main() {
	vec4 color = vec4(0.883, 0.893, 0.945, 0.0);
	
	const float dottedLineLength = 8;
	float dottedLineWidth = dottedLineLength / 2.0;

	vec2 worldCoords = (gl_FragCoord.xy - screenSize * 0.5) / scale + cameraPos;

	color.a = step(abs(worldCoords.x), width);
	
	if(!modified(color.a)){
		color.a = step(abs(worldCoords.y), width);
		
		if(!modified(color.a)){
			color.a = step(mod(abs(worldCoords.x), spacing), width) * subLineGrayVal;
			
			if(modified(color.a)){
				color.a *= 1.0 - step(mod(abs(worldCoords.y), dottedLineLength), dottedLineWidth);
			}else{
				color.a = step(mod(abs(worldCoords.y), spacing), width) * subLineGrayVal;
				
				if(modified(color.a)){
					color.a *= 1.0 - step(mod(abs(worldCoords.x), dottedLineLength), dottedLineWidth);
				}
			}
		}
	}
	
	color.r = color.a;
	color.g = color.a;
	color.b = color.a;
	
//	color.rg *= (gl_FragCoord.xy / screenSize);

	gl_FragColor = color;
}
