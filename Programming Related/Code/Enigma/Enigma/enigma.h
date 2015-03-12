#pragma once
/*
Majed Monem
Encryption/Decryption class
*/

#include <string>
#include <iostream>

class enigma
{
public:
	enigma();
	~enigma();

	void encrypt();
	void decrpyt();

	std::string getRotorOne();
	void setRotorOne(std::string str);

	std::string getStaticrOne();
	void setStaticrOne(std::string str);

	std::string alphabet;
	std::string rotorOne;
	std::string staticrOne;
	std::string reflector;
	std::string getReflector();
	std::string getAlphabet();
	void offset(int dir);
	void setReflector(std::string str);
	int getIndex(char k);
};