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
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <ostream>
#include <vector>
#include <algorithm>

namespace TP3
{
	// implementation du constructeur
	DisqueVirtuel::DisqueVirtuel(){
		bd_FormatDisk();
	}

	// implementation du destructeur
	DisqueVirtuel::~DisqueVirtuel(){
		for(int index = BASE_BLOCK_INODE; index < 24 ;index++){
			delete m_blockDisque[index].m_inode;
		}
	}

	//creation de la premiere fonction
	int DisqueVirtuel::bd_FormatDisk(){

		try{ //pour la capture des erreur
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

			m_blockDisque[fistPos].m_dirEntry.clear();       //j,efface tout le veteur
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
			// je prend le premier inode libre et le premier bloc libre
			int indexInodeLibre = trouverPremierINodeLibre();// inode 
			int indexIBlockLibre = trouverPremierBlocLibre(); //
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

		

	

	int DisqueVirtuel::bd_mkdir(const std::string &p_DirName) {
        
       std::pair<std::string, std::string> result = this->getFichierDossier(p_DirName);
	   std::string nomRepos = result.first;
	   std::string nomFichier = result.second;

	   if (RepertoireEstExistant(nomRepos)) {
            // je cree le repertoire  alors si le parent existe normalement
			// je prend le premier indode de libre
			int indexInodeLibre = trouverPremierINodeLibre() ;
			//je recherche l,index de inode de libre  enfet
			int indexIBlockLibre = trouverPremierBlocLibre();
			//je recherche l,index de libre enfet
			creationDuRepertoire(m_InodeRacine, nomFichier,indexInodeLibre,indexIBlockLibre);
			return 1;
        }

		return 0;
    }


	std::string DisqueVirtuel::bd_ls(const std::string &p_DirLocation) {
     // avant de le faire il faut verifier si le fichier existe normalement 
                std::string formatageSortie = "";
 		std::ostringstream FormatageOS;

		std::string cheminRecherche = p_DirLocation;

		
		if (p_DirLocation.back() != '/') {
			cheminRecherche += '/';  // Comparer avec le chemin normalisé
		    }
	 	std::pair<std::string, std::string> result = this->getFichierDossier(cheminRecherche);
		std::string nomRepos = result.first;
		std::string nomFichier = result.second;

		    
		auto estExistant = RepertoireEstExistant(nomRepos);
		if(estExistant){
			int index = 0;
        	auto repos = m_blockDisque[m_InodeRacine->st_block].m_dirEntry;
			//creation element affichage
			std::string Size = "Size:\t" ;
			std::string inode = "inode:\t" ;
			std::string nlink = "nlink:\t" ;
			std::string indentation = "         ";
			
       		while (index < repos.size()) {
				int numeroInode = m_blockDisque[m_InodeRacine->st_block].m_dirEntry[index]->m_iNode;
				auto intermediare = numeroInode + BASE_BLOCK_INODE;
				std::string typeSelec = (m_blockDisque[intermediare].m_inode->st_mode == S_IFDIR)? "d" : "-";
				std::string NomRetourner = m_blockDisque[m_InodeRacine->st_block].m_dirEntry[index]->m_filename;
				std::string formatageDuNom = indentation.substr(0, indentation.size()-NomRetourner.length()) + NomRetourner + " ";
				FormatageOS << typeSelec + formatageDuNom;
				FormatageOS << Size << m_blockDisque[numeroInode + BASE_BLOCK_INODE].m_inode->st_size << " ";
				FormatageOS << inode << m_blockDisque.at(numeroInode + BASE_BLOCK_INODE).m_inode->st_ino << " ";
				FormatageOS << nlink << m_blockDisque[numeroInode + BASE_BLOCK_INODE].m_inode->st_nlink << std::endl;
				index++;
			}
			formatageSortie = FormatageOS.str();
 		}
  		return formatageSortie;
	}

	int DisqueVirtuel::trouverPremierBlocLibre() {
			int indexIBlockLibre = 0;
		while (indexIBlockLibre < N_BLOCK_ON_DISK) { 
				if (m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[indexIBlockLibre]) {
					m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[indexIBlockLibre] = false;
					std::cout << "UFS: Saisie bloc  " << indexIBlockLibre << std::endl ;
					 break;
				}

				indexIBlockLibre++;
			} 
			return indexIBlockLibre;     
    }

	int DisqueVirtuel::trouverPremierINodeLibre() {
		int indexInodeLibre = ROOT_INODE;
		while (indexInodeLibre < N_INODE_ON_DISK) { 
				if (m_blockDisque[FREE_INODE_BITMAP].m_bitmap[indexInodeLibre]) {
					  m_blockDisque[FREE_INODE_BITMAP].m_bitmap[indexInodeLibre] = false;
        			  std::cout << "UFS: Saisie i-node " << indexInodeLibre << std::endl;
					  break;
				   }
				indexInodeLibre++;  
			}
			return indexInodeLibre;
    }

	void DisqueVirtuel::creationDuRepertoire(iNode* inodeParent, const std::string &nomFichier, int indexInodeLibre, int indexIBlockLibre){
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

	std::pair<std::string, std::string> DisqueVirtuel::getFichierDossier(const std::string &nomFichier) {
			std::size_t positionFile= nomFichier.find_last_of("/");
			std::string NomFichier = nomFichier.substr(positionFile + 1);
			std::string CheminDossier = nomFichier.substr(0, positionFile);
			std::size_t positionDossier= CheminDossier.find_last_of("/");
			std::string NomDossier = CheminDossier.substr(positionDossier + 1);
		return {NomDossier, NomFichier};
		}



	int DisqueVirtuel::bd_rm(const std::string &p_Filename) {
	    int success = 0;

	    std::string directoryName = p_Filename.substr(p_Filename.find_last_of("/") + 1);
	    bool elementFound = false;
	    dirEntry* removalDirEntry;
	    int m_sourceINodeNumber;

	    // Recherche de l element
	    for (int i = N_INODE_ON_DISK + BASE_BLOCK_INODE; i < N_BLOCK_ON_DISK; i++) {
		for (auto& entry : m_blockDisque[i].m_dirEntry) {
		    if (entry->m_filename.compare(directoryName) == 0) {
		        int inodeNumber = entry->m_iNode;
		        m_InodeRacine = m_blockDisque[inodeNumber + BASE_BLOCK_INODE].m_inode;
		        removalDirEntry = entry;
		        m_sourceINodeNumber = m_blockDisque[i].m_dirEntry[0]->m_iNode;
		        elementFound = true;
		        break;
		    }
		}
		if (elementFound) break;
	    }

	    if (!elementFound) return success;

	    // Verifier le type et supprimer l element
	    if (m_InodeRacine->st_mode == S_IFDIR) {

		Block& directory = m_blockDisque[m_InodeRacine->st_block];
		bool isEmpty = true;
		if (directory.m_dirEntry.size() == 2) {
		    for (auto& dirEntry : directory.m_dirEntry) {
		        if (dirEntry->m_filename != "." && dirEntry->m_filename != "..") {
		            isEmpty = false;
		            break;
		        }
		    }
		} else {
		    isEmpty = false;
		}

		if (isEmpty) {
		    m_InodeRacine->st_nlink--;
		    if (m_InodeRacine->st_nlink == 1) {
		        m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[m_InodeRacine->st_block] = true;
		        m_blockDisque[FREE_INODE_BITMAP].m_bitmap[m_InodeRacine->st_ino] = true;
		
			std::cout << "UFS: Relache i-node " << m_InodeRacine->st_ino << std::endl;
			std::cout << "UFS: Relache bloc " << m_InodeRacine->st_block << std::endl;
		

		        auto& directory = m_blockDisque[m_sourceINodeNumber + BASE_BLOCK_INODE].m_inode->st_block;
		        auto& dirEntries = m_blockDisque[directory].m_dirEntry;
		        dirEntries.erase(std::remove(dirEntries.begin(), dirEntries.end(), removalDirEntry), dirEntries.end());

		        iNode* sourceINode = m_blockDisque[m_sourceINodeNumber + BASE_BLOCK_INODE].m_inode;
		        sourceINode->st_nlink--;
		        sourceINode->st_size -= 28;
		    }
		    success = 1;
		}
	    } else if (m_InodeRacine->st_mode == S_IFREG) {
		m_InodeRacine->st_nlink--;
		if (m_InodeRacine->st_nlink == 0) {
		    m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[m_InodeRacine->st_block] = true;
		    m_blockDisque[FREE_INODE_BITMAP].m_bitmap[m_InodeRacine->st_ino] = true;
		    std::cout << "UFS: Relache i-node " << m_InodeRacine->st_ino << std::endl;

		    auto& directory = m_blockDisque[m_sourceINodeNumber + BASE_BLOCK_INODE].m_inode->st_block;
		    auto& dirEntries = m_blockDisque[directory].m_dirEntry;
		    dirEntries.erase(std::remove(dirEntries.begin(), dirEntries.end(), removalDirEntry), dirEntries.end());

		    iNode* sourceINode = m_blockDisque[m_sourceINodeNumber + BASE_BLOCK_INODE].m_inode;
		    sourceINode->st_size -= 28;
		}
		success = 1;
	    }

	    return success;
	}





}//Fin du namespace