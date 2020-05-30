#pragma once

typedef struct sdfParams
{
	float lvShift; //lower value shift
	int texSize; //size of texture, here cube like equal
    sdfParams() { lvShift = 0; texSize = 0; }
    sdfParams(float l, int tex) { lvShift = l; texSize=tex; }
    sdfParams(sdfParams* param) { lvShift = param->lvShift; texSize= param->texSize; }
};

typedef struct
{
	float density;
	float brightness; 
	float transferOffset;
	float transferScale;
	float time;
}   visParams;


