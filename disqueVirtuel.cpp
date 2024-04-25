/**
 * \file disqueVirtuel.cpp
 * \brief Implémentation d'un disque virtuel.
 * \author ?
 * \version 0.1
 * \date  2021
 *
 *  Travail pratique numéro 3
 *
 */

#include "disqueVirtuel.h"
#include <iostream>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
//vous pouvez inclure d'autres librairies si c'est nécessaire

namespace TP3
{
	DisqueVirtuel::DisqueVirtuel(){
		bd_FormatDisk();
	}
	DisqueVirtuel::~DisqueVirtuel(){
		for(int index = BASE_BLOCK_INODE; index < 24 ;index++){
			delete m_blockDisque[index].m_inode;
		}
	}

	//creation de la premiere fonction

	int DisqueVirtuel::bd_FormatDisk(){

		try{
			m_blockDisque.clear(); // je vide mon vecteur dabord
			for(int idx = 0 ; idx < 128;idx++){
				m_blockDisque.push_back(Block()); // je creer les 28 bloc vide du disqeu
			}
             //initialiser le bitmap des blocs libres dans le bloc 2
			 Block blockDeBitmap = Block(S_IFBL);// creation bloc pour bitmap bloc libre
			 // je suit entrein de construire mon bitmap pou1r le bloc 2

			for(int index = 0; index < 24 ;index++){
				blockDeBitmap.m_bitmap.push_back(false);
			}

			for(int index = 24; index < 128 ;index++){
				blockDeBitmap.m_bitmap.push_back(true);
			}
			m_blockDisque[2] = blockDeBitmap;

			// maintenant je creer le bloc bitmap pour les inodes
			Block blockDeInode = Block(S_IFIL);
			blockDeInode.m_bitmap.push_back(false);
            int ind_inode = 1;

			while(ind_inode < 20){
				blockDeInode.m_bitmap.push_back(true);
				ind_inode++;
			}

			m_blockDisque[3] = blockDeInode;// j,assigne la valeur au disque
             int debut = 4;
			 ind_inode = debut;

			 while(ind_inode < 24){
				m_blockDisque[ind_inode] = Block(S_IFIN);// je defini le type de bloc
				m_blockDisque[ind_inode].m_inode = new iNode(ind_inode - debut,S_IFREG,0,0,0);// j,initialise l,inode
				ind_inode++; // je le fait pour mes 19 blocs
			 }

			m_blockDisque[24] = Block(S_IFDE);
			m_blockDisque[5].m_inode->st_mode = S_IFDIR;
			m_blockDisque[5].m_inode->st_size = 56;
			m_blockDisque[5].m_inode->st_block= 24;
			m_blockDisque[5].m_inode->st_nlink = 2;

            
			int fistPos = -1;

			for(int pos =0; pos < m_blockDisque[2].m_bitmap.size(); pos++){
				if(m_blockDisque[2].m_bitmap[pos]){
					fistPos = pos;
					break;
				}
 			}

			auto &directorieEntrie = m_blockDisque[fistPos].m_dirEntry;
			for(auto *entry : directorieEntrie){
				delete entry;
			}

			auto dirRacine = new dirEntry(1, ".");
			auto dirParent = new dirEntry(1, "..");

				m_blockDisque[fistPos].m_dirEntry.clear(); //j,efface tout le veteur
				m_blockDisque[fistPos].m_dirEntry.push_back(dirRacine);
				m_blockDisque[fistPos].m_dirEntry.push_back(dirParent);
				m_blockDisque[3].m_bitmap[1] = false;
				m_blockDisque[2].m_bitmap[fistPos] = false;
				
				
				std:: cout << "UFS: Saisie i-node " << 1 << std::endl;
				std:: cout << "UFS: Saisie bloc " << fistPos << std::endl;

		}catch(const std::exception& e){
			std::cerr << "Erreur: " << e.what() <<std::endl;
			return 0;
		}

		return 1; 
	}

	int TP3::DisqueVirtuel::bd_create(const std::string& p_FileName){
			
        std::pair<std::string, std::string> result = this->getFichierDossier(p_FileName);
		std::string nomRepos = result.first;
		std::string nomFichier = result.second;

		
		bool Repovalide = this->RepertoireEstExistant(nomRepos);
		bool FichierValide =  this->RepertoireEstExistant(nomFichier);

		if(Repovalide && !FichierValide){

			int indexInodeLibre = ROOT_INODE;
			int indexIBlockLibre = 0;

			//je recherche l,index de inode de libre  enfet

			while (indexInodeLibre < N_INODE_ON_DISK) { 
				if (m_blockDisque[FREE_INODE_BITMAP].m_bitmap[indexInodeLibre]) {
					  m_blockDisque[FREE_INODE_BITMAP].m_bitmap[indexInodeLibre] = false;
        			  std::cout << "UFS: Saisie i-node " << indexInodeLibre << std::endl;
					  break;
				   }
				indexInodeLibre++;  
			}

			//je recherche l,index de libre enfet
			while (indexIBlockLibre < N_BLOCK_ON_DISK) { 
				if (m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[indexIBlockLibre]) {
					m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[indexIBlockLibre] = false;
					std::cout << "UFS: Saisie bloc  " << indexIBlockLibre << std::endl ;
					 break;
				}

				indexIBlockLibre++;
			}
			// recuperation terminer

			 iNode *fichierInode = new iNode(indexInodeLibre, S_IFREG, 1, 0, indexIBlockLibre);
			 m_blockDisque[indexInodeLibre + BASE_BLOCK_INODE].m_inode = fichierInode;
			 dirEntry *my_dirEntry = new dirEntry(indexInodeLibre, nomFichier);
			 m_blockDisque[m_InodeRacine->st_block].m_dirEntry.push_back(my_dirEntry);
			 int InodePere = m_blockDisque[m_InodeRacine->st_block].m_dirEntry[0]->m_iNode;
             auto monInode =  m_blockDisque[InodePere + BASE_BLOCK_INODE].m_inode;
			 monInode->st_size += 28;
			 return 1;
		}
		return 0;

	}


std::pair<std::string, std::string> DisqueVirtuel::getFichierDossier(const std::string &nomFichier) {
	std::size_t positionFile= nomFichier.find_last_of("/");
	std::string NomFichier = nomFichier.substr(positionFile + 1);
	std::string CheminDossier = nomFichier.substr(0, positionFile);
	std::size_t positionDossier= CheminDossier.find_last_of("/");
	std::string NomDossier = CheminDossier.substr(positionDossier + 1);
return {NomDossier, NomFichier};
}

bool DisqueVirtuel::RepertoireEstExistant(const std::string &nomRepertoire) {
	if (nomRepertoire.size() == 0) {
    m_InodeRacine = m_blockDisque[5].m_inode;
    return true;
	}
	int i = N_INODE_ON_DISK + BASE_BLOCK_INODE;
	while (i < N_BLOCK_ON_DISK) {
		int index = 0;
		while (index < m_blockDisque[i].m_dirEntry.size()) {
			if (m_blockDisque[i].m_dirEntry[index]->m_filename.compare(nomRepertoire) == 0) {
				int numeroInode = m_blockDisque[i].m_dirEntry[index]->m_iNode;
				int tmp = numeroInode + BASE_BLOCK_INODE;
				this->m_InodeRacine = m_blockDisque[tmp].m_inode;
				return true;
			}

			index++;
		}
		i++;
	}

	return false; // Retourne false si acun repertoire trouve dans le bloc
}

int DisqueVirtuel::bd_mkdir(const std::string &p_DirName) {
        
       std::pair<std::string, std::string> result = this->getFichierDossier(p_DirName);
	   std::string nomRepos = result.first;
	   std::string nomFichier = result.second;

	   if (RepertoireEstExistant(nomRepos)) {
            // je cree le repertoire  alors si le parent existe normalement
			// je prend le premier indode de libre
			int indexInodeLibre = ROOT_INODE;
			int indexIBlockLibre = 0;
			//je recherche l,index de inode de libre  enfet
			while (indexInodeLibre < N_INODE_ON_DISK) { 
				if (m_blockDisque[FREE_INODE_BITMAP].m_bitmap[indexInodeLibre]) {
					  m_blockDisque[FREE_INODE_BITMAP].m_bitmap[indexInodeLibre] = false;
        			  std::cout << "UFS: Saisie i-node " << indexInodeLibre << std::endl;
					  break;
				   }
				indexInodeLibre++;  
			}
			//je recherche l,index de libre enfet
			while (indexIBlockLibre < N_BLOCK_ON_DISK) { 
				if (m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[indexIBlockLibre]) {
					m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[indexIBlockLibre] = false;
					std::cout << "UFS: Saisie bloc  " << indexIBlockLibre << std::endl ;
					 break;
				}

				indexIBlockLibre++;
			}

            iNode *repertoireInode = new iNode(indexInodeLibre, S_IFDIR, 1, 28, indexIBlockLibre);
			int nouvelleIndex = indexInodeLibre + BASE_BLOCK_INODE;
			m_blockDisque[nouvelleIndex].m_inode = repertoireInode;
			m_blockDisque[indexIBlockLibre] = Block(S_IFDE);
			dirEntry *monRepertoireParent = new dirEntry(m_InodeRacine->st_ino, "..");
			dirEntry *reposCourant = new dirEntry(repertoireInode->st_ino, ".");
			dirEntry *monRepos = new dirEntry(indexInodeLibre, nomFichier);
			m_blockDisque[indexIBlockLibre].m_dirEntry.push_back(reposCourant);
			m_blockDisque[indexIBlockLibre].m_dirEntry.push_back(monRepertoireParent);
			m_blockDisque[m_InodeRacine->st_block].m_dirEntry.push_back(monRepos);
			repertoireInode->st_nlink++;
			m_InodeRacine->st_nlink++;
			repertoireInode->st_size += 28;
			m_InodeRacine->st_size += 28;

			return 1;
        }

		return 0;
    }


	std::string DisqueVirtuel::bd_ls(const std::string &p_DirLocation) {
     // avant de le faire il faut verifier si le fichier existe normalement 
 		std::ostringstream FormatageOS;
	 	std::pair<std::string, std::string> result = this->getFichierDossier(p_DirLocation);
		std::string nomRepos = result.first;
		std::string nomFichier = result.second;
		auto estExistant = RepertoireEstExistant(nomRepos);
		if(estExistant){
			int index = 0;
        	auto repos = m_blockDisque[m_InodeRacine->st_block].m_dirEntry;
       		while (index < repos.size()) {
				int numeroInode = m_blockDisque[m_InodeRacine->st_block].m_dirEntry[index]->m_iNode;
				std::string typeSelec = (m_blockDisque[numeroInode + BASE_BLOCK_INODE].m_inode->st_mode == S_IFDIR)? "d" : "-";
				std::string indentation = "         ";
				std::string NomRetourner = m_blockDisque[m_InodeRacine->st_block].m_dirEntry[index]->m_filename;
				std::string formatageDuNom = indentation.substr(0, indentation.size()-NomRetourner.length()) + NomRetourner + " ";
				FormatageOS << typeSelec + formatageDuNom;
				FormatageOS << "Size:\t" << m_blockDisque[numeroInode + BASE_BLOCK_INODE].m_inode->st_size << " ";
				FormatageOS << "inode:\t" << m_blockDisque.at(numeroInode + BASE_BLOCK_INODE).m_inode->st_ino << " ";
				FormatageOS << "nlink:\t" << m_blockDisque[numeroInode + BASE_BLOCK_INODE].m_inode->st_nlink << std::endl;
				index++;

			}
 		}
  			return FormatageOS.str();
	}

}//Fin du namespace
