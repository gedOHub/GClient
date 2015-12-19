#include "StdAfx.h"
#include "TagGenerator.h"


TagGenerator::TagGenerator(void)
{
	this->MIN = 1000;
	this->MAX = 65534;

	this->skaitliukas = MIN;
}
int TagGenerator::GetTag(){
	if (this->skaitliukas >= MAX)
		this->Reset();

	/* TODO:
	Prideti apsauga ir patirkinima ar yra jau toks tagas isduotas
	*/

	this->skaitliukas = this->skaitliukas + 1;
	return this->skaitliukas;
}

void TagGenerator::Reset(){
	this->skaitliukas = this->MIN;
}
