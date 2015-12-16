#include "StdAfx.h"
#include "TagGenerator.h"


TagGenerator::TagGenerator(void)
{
	this->skaitliukas = 0;
}
int TagGenerator::GetTag(){
	this->skaitliukas = this->skaitliukas + 1;
	return this->skaitliukas;
}
