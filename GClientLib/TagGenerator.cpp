#include "StdAfx.h"
#include "TagGenerator.h"

using namespace GClientLib;

GClientLib::TagGenerator::TagGenerator(void)
{
	this->MIN = 1000;
	this->MAX = 65534;

	this->skaitliukas = MIN;
}

int GClientLib::TagGenerator::GetTag(){
	if (this->skaitliukas >= MAX)
		//this->Reset();

	/* TODO:
	Prideti apsauga ir patirkinima ar yra jau toks tagas isduotas
	*/

	this->skaitliukas = this->skaitliukas + 1;
	return this->skaitliukas;
}

void GClientLib::TagGenerator::Reset(){
	this->skaitliukas = this->MIN;
}
