/**
 * \file block.cpp
 * \brief Implémentation d'un bloc.
 * \author ?
 * \version 0.1
 * \date  2021
 *
 *  Travail pratique numéro 3
 *
 */

#include "block.h"
//vous pouvez inclure d'autres librairies si c'est nécessaire

namespace TP3
{
	// IMPLEMENTATION DU CONSTRUCTEUR PAR DEFAUT
	Block::Block(){

	}
	// IMPLEMENTATION DU CONstructeur avec parametre
	Block::Block(size_t typeData ){
		m_type_donnees = typeData;
	}
	// IMPLEMENTATION DU destructeur

	Block::~Block(){
		for(auto val: m_dirEntry){
			delete val;
		}
	}
}

//Fin du namespace




