#include "TagGenerator.h"

using namespace GClientLib;

GClientLib::TagGenerator::TagGenerator(SocketToObjectContainer^ con)
{
	this->MIN = 1000;
	this->MAX = 65534;

	this->skaitliukas = MIN;
	this->container = con;
}

int GClientLib::TagGenerator::GetTag(){
	if (this->skaitliukas >= MAX)
		this->Reset();

	// Didinu skaitliuka
	this->skaitliukas = this->skaitliukas + 1;

	
	// Tikrinu ar jau toks yra naudojamas
	while (!this->isFree(this->skaitliukas))
	{
		// Didinu skaitliuka
		this->skaitliukas = this->skaitliukas + 1;
	}
	

	return this->skaitliukas;
}

void GClientLib::TagGenerator::Reset(){
	this->skaitliukas = this->MIN;
}

bool  GClientLib::TagGenerator::isFree(int tag) {
	// Ieskau objekto
	gNetSocket^ obj = this->container->FindByTag(tag);

	if (obj == nullptr)
	{
		// Neradau, galima naudoti
		return true;
	} else
	{
		// Radau, negalima naudoti
		return false;
	}
}
