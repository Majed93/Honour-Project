#pragma once
/*
Majed Monem 2014/15 Graphical Enigma Simulator Honours Project
Encryption/Decryption class
*/
#include "enigma.h"
#include <string>
#include <iostream>

//Constructor for Enigma object
enigma::enigma()
{
	alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
}

//Destructor for Enigma object
enigma::~enigma()
{

}

//Encryption occurs here
char enigma::encrypt(int index)
{
	
	char_rOne = getRotorOne().at(index);
	char_reflect = getReflector().at(getIndex(char_rOne));
	st_newchar = getRotorOne().find(char_reflect, 0);
	char_letter = getAlphabet().at(st_newchar);
	
	//DEBUGGING
	std::cout << "rone " << char_rOne << std::endl;
	std::cout << "reflect " << char_reflect << std::endl;
	std::cout << "newrotor " << st_newchar << std::endl;
	std::cout << "letter " << char_letter << std::endl;
	std::cout << " " << std::endl;
	return char_letter;// machine.getRotorOne().at(st_newchar);
}

//Decryption occurs here
char enigma::decrypt(int index, char k)
{
	st_rotorone = getRotorOne().find(k, 0);
	char_rOne = getRotorOne().at(index);
	st_newreflect = getReflector().find(char_rOne, 0);
	char_inrOne = getAlphabet().at(st_newreflect);
	st_newchar = getRotorOne().find(char_inrOne, 0);
	char_letter = getAlphabet().at(st_newchar);

	//DEBUGGING
	//std::cout << "newrotor " << st_rotorone << std::endl;//NOT NEEDED
	std::cout << "rOne " << char_rOne << std::endl;
	std::cout << "newreflect " << st_newreflect << std::endl;
	std::cout << "in rotor one " << char_inrOne << std::endl;
	std::cout << "Rotor one index " << st_newchar << std::endl;
	std::cout << "letter " << char_letter << std::endl;
	std::cout << " " << std::endl;

	return char_letter;// machine.getAlphabet().at(st_newchar);


}

//Returns the alphabet string
std::string enigma::getAlphabet()
{
	return alphabet;
}

//Returns the current rotor one string
std::string enigma::getRotorOne()
{
	return rotorOne;
}

//Sets rotor one string
void enigma::setRotorOne(std::string str)
{
	rotorOne = str;
}

//Returns static rotor string
std::string enigma::getStaticrOne()
{
	return staticrOne;
}

//Sets static rotor string
void enigma::setStaticrOne(std::string str)
{
	staticrOne = str;
}

//Return reflector string
std::string enigma::getReflector()
{
	return reflector;
}

//Sets reflector string
void enigma::setReflector(std::string str)
{
	reflector = str;
}

//Moves the rotor position. i.e the string moves up one index location.
void enigma::offset(int dir)
{
	std::string newcode;
	char first = NULL;

	//FORWARD
	if (dir > 0)
	{

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
	}

	//BACKWARDS
	if (dir < 0)
	{
		for (int i = 0; i < rotorOne.size(); i++)
		{
			char temp = NULL;
			if (i == 25)
			{
				first = rotorOne.at(0);
				newcode += first;
			}
			else
			{
				temp = rotorOne.at(i + 1);
				newcode += temp;
			}
		}
	}
	
	setRotorOne(newcode);
}

//This is bad of doing it, change later if time.
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

//Reset variables
void enigma::reset()
{
	st_rotorone = 0;
	char_rOne = ' ';
	char_reflect = ' ';
	char_letter = ' ';
	st_newreflect = 0;
	char_inrOne = ' ';
	st_newchar = 0;
}