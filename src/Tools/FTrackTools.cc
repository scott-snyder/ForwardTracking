#include "FTrackTools.h"

#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include <sstream>
#include <fstream>
#include <iostream>


using namespace FTrack;

//TODO: don't assume a fileending of .root. Be more flexible! (Maybe even write a more general routine dealing with any file


void FTrack::setUpRootFile( std::string rootNamePath, std::string treeName ,  std::set<std::string> branchNames ){
   
   
   std::string fileNamePath = rootNamePath.substr( 0 , rootNamePath.find_last_of(".")  );
   

   
   ifstream rf ((fileNamePath + ".root").c_str());       //rf for RootFile
   if (rf) { // The file already exists
   
    int i=0;
    while (rf){         //Try adding a number starting from 1 to the filename until no file with this name exists and use this.
      
      rf.close();
      i++;
      rf.open((fileNamePath + intToString(i) + ".root").c_str());
      
    }
    rename ( (fileNamePath + ".root").c_str() , (fileNamePath + intToString(i) +".root").c_str());      //renames the file in the way,so that our new file can have it's name
    //and not ovrewrite it.
    
   }
   
   float x = 0;
   
   TFile* myRootFile = new TFile((fileNamePath + ".root").c_str(), "RECREATE");        //Make new file, if there is an old one
   TTree* myTree;
   
   myTree = new TTree(treeName.c_str(),"My tree"); //make a new tree
   
   //create the branches:
   
   std::set < std::string >::iterator it;
   
   std::cout << "\n\n nBranchNames: " << branchNames.size() << "\n\n";
   
   for ( it = branchNames.begin() ; it != branchNames.end() ; it++ ){
      
   
      myTree->Branch( (*it).c_str() , &x );
      
   }
   
   
   myTree->Write("",TObject::kOverwrite);
   myRootFile->Close();
   
}


std::string FTrack::intToString (int i){
   
   std::ostringstream sin;
   sin << i;
   return sin.str();
   
}


void FTrack::saveToRoot( std::string rootFileName, std::string treeName , std::map < std::string , float > map_name_data ){


   
   std::map < std::string , float >::iterator it;

   
   TFile*   myRootFile = new TFile( rootFileName.c_str(), "UPDATE"); //add values to the root file
   TTree*   myTree = dynamic_cast <TTree*>( myRootFile->Get( treeName.c_str()) );




   for( it = map_name_data.begin() ; it != map_name_data.end() ; it++){
      

        
      myTree->SetBranchAddress( it->first.c_str(), & it->second );   
         
                               
                                    
   }


   myTree->Fill();
   myTree->Write("",TObject::kOverwrite);

   myRootFile->Close();

}


bool FTrack::compare_z( EVENT::TrackerHit* a, EVENT::TrackerHit* b ){
   
   return ( fabs(a->getPosition()[2]) < fabs( b->getPosition()[2]) ); //compare their z values
   
}







