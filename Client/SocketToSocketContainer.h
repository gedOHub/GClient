#pragma once
ref class SocketToSocketContainer
{
private:
	// Sujungimu sarasas
	cliext::list<cliext::pair<int, int>> sarasas;
public:
	SocketToSocketContainer(void);
	void Add(int, int);
	int Find(int);
	void Delete(int);
};

