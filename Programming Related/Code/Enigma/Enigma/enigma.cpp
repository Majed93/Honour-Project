#pragma once
/*
Majed Monem
Encryption/Decryption class
*/
#include "enigma.h"
#include <string>
#include <iostream>


enigma::enigma()
{
	alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
}

enigma::~enigma()
{
}

void enigma::encrypt()
{

}

void enigma::decrpyt()
{

}

std::string enigma::getAlphabet()
{
	return alphabet;
}

std::string enigma::getRotorOne()
{
	return rotorOne;
}

void enigma::setRotorOne(std::string str)
{
	rotorOne = str;
}

std::string enigma::getReflector()
{
	return reflector;
}

void enigma::setReflector(std::string str)
{
	reflector = str;
}

void enigma::offset()
{
	std::string newcode;
	char first = NULL;
	for (int i = 0; i < rotorOne.size(); i++)
	{
		char temp = NULL;
		if (i == 0)
		{
			first = rotorOne.at(25);
			newcode = first;
		}
		else
		{
			temp = rotorOne.at(i - 1);
			newcode += temp;
		}
	}
	//std::cout << "new " << newcode << std::endl;
	setRotorOne(newcode);
}
int enigma::getIndex(char k)
{
	int character = NULL;
	switch (k)
	{
	case 'A':
		character = 0;
		break;

	case 'B':
		character = 1;
		break;

	case 'C':
		character = 2;
		break;

	case 'D':
		character = 3;
		break;

	case 'E':
		character = 4;
		break;

	case 'F':
		character = 5;
		break;

	case 'G':
		character = 6;
		break;

	case 'H':
		character = 7;
		break;

	case 'I':
		character = 8;
		break;

	case 'J':
		character = 9;
		break;

	case 'K':
		character = 10;
		break;

	case 'L':
		character = 11;
		break;

	case 'M':
		character = 12;
		break;

	case 'N':
		character = 13;
		break;

	case 'O':
		character = 14;
		break;

	case 'P':
		character = 15;
		break;

	case 'Q':
		character = 16;
		break;

	case 'R':
		character = 17;
		break;

	case 'S':
		character = 18;
		break;

	case 'T':
		character = 19;
		break;

	case 'U':
		character = 20;
		break;

	case 'V':
		character = 21;
		break;

	case 'W':
		character = 22;
		break;

	case 'X':
		character = 23;
		break;

	case 'Y':
		character = 24;
		break;

	case 'Z':
		character = 25;
		break;

	default:
		break;

	}

	return character;
}