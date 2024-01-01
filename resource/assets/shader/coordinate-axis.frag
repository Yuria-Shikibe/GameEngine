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
	
	color.a *= 0.9f;
	
	color.r = color.a;
	color.g = color.a;
	color.b = color.a;
	
	float newA = 0.0f;
	
	if(!modified(newA)){
		newA = step(abs(gl_FragCoord.x - screenSize.x * 0.5f), 1.0f) * 0.4f;
		
		if(!modified(newA)){
			newA = step(abs(gl_FragCoord.y - screenSize.y * 0.5f), 1.0f) * 0.4f;
		}
		
		if(modified(newA)){
			color.r = 0.3f;
			color.g = 0.961f;
			color.b = 0.671f;
		}
		
		color.a = max(color.a, newA);
	}
	
	if(!modified(color.a)){
		color = vec4(0.42f, 0.55f, 1.0f, 0.12f);
	}
	
//	color.rg *= (gl_FragCoord.xy / screenSize);

	gl_FragColor = color;
}
