#pragma once
/*
Majed Monem 2014/15 Graphical Enigma Simulator Honours Project
Encryption/Decryption class header
*/

#include <string>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class enigma
{
public:
	enigma();
	~enigma();

	char encrypt(int index);
	char decrypt(int index, char k);

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
	void reset();

	std::size_t st_rotorone;
	char char_rOne;
	char char_reflect;
	char char_letter;
	std::size_t st_newreflect;
	char char_inrOne;
	std::size_t st_newchar;

};