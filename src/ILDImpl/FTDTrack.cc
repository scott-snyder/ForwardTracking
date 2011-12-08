#include "FTDTrack.h"
#include "FTrackTools.h"

// Root, for calculating the chi2 probability. 
#include "Math/ProbFunc.h"
#include <algorithm>


using namespace FTrack;


MarlinTrk::IMarlinTrkSystem* FTDTrack::_trkSystem ;

FTDTrack::FTDTrack(){
   
   
 
   _lcioTrack = new TrackImpl();
   
   
}

FTDTrack::FTDTrack( std::vector< IHit* > hits ){
   
   
   
   _lcioTrack = new TrackImpl();
   
   for( unsigned i=0; i < hits.size(); i++ ){
      
      addHit( hits[i] );
      
      
   }
   
}


FTDTrack::FTDTrack( const FTDTrack& f ){

   //make a new copied lcio track
   _lcioTrack = new TrackImpl( *f._lcioTrack );
   
   
   _hits = f._hits;
   _chi2Prob = f._chi2Prob;

}

FTDTrack & FTDTrack::operator= (const FTDTrack & f){
   
   
   //make a new copied lcio track
   _lcioTrack = new TrackImpl( *f._lcioTrack );
   
   
   _hits = f._hits;
   _chi2Prob = f._chi2Prob;
   
   return *this;
   
}



void FTDTrack::addHit( IHit* hit ){
   
   
   
   // add the hit
   IFTDHit* ftdHit = dynamic_cast< IFTDHit* >( hit );
   
   if ( ftdHit != NULL ){
      
      _hits.push_back( ftdHit );
      
      // and sort the track again
      sort( _hits.begin(), _hits.end(), compare_z ); //TODO: maybe make this more flexible for different sorting
      
      
      _lcioTrack->addHit( ftdHit->getTrackerHit() );
      
   }
   //TODO: throw exception, if cast was not succesfull. Question: is there a better way of dealing with this?
   
}



void FTDTrack::initialiseFitter( const std::string& systemType,  
                               const gear::GearMgr* mgr , 
                               const std::string& options ,
                               const bool MSOn ,
                               const bool ElossOn ,
                               const bool SmoothOn
                             ) {
   
   
   // set upt the geometry
   _trkSystem =  MarlinTrk::Factory::createMarlinTrkSystem( systemType , mgr , options ) ;
   
   // set the options   
   _trkSystem->setOption( MarlinTrk::IMarlinTrkSystem::CFG::useQMS,        MSOn ) ;       //multiple scattering
   _trkSystem->setOption( MarlinTrk::IMarlinTrkSystem::CFG::usedEdx,       ElossOn) ;     //energy loss
   _trkSystem->setOption( MarlinTrk::IMarlinTrkSystem::CFG::useSmoothing,  SmoothOn) ;    //smoothing
   
   // initialise the tracking system
   _trkSystem->init() ;
   
   
   
}




void FTDTrack::fit(){
   

   
   //Create a new MarlinTrack
   MarlinTrk::IMarlinTrack* marlin_trk = _trkSystem->createTrack();
   
   
   EVENT::TrackerHitVec trkHits = _lcioTrack->getTrackerHits() ;        
   
   // sort the hits
   sort( trkHits.begin(), trkHits.end(), compare_TrackerHit_z );
   
   // now at [0] is the hit with the smallest |z| and at [1] is the one with a bigger |z| and so on
   // So the direction of the hits when following the index from 0 on is:
   // from inside out: from the IP into the distance. 
   // (It is important to keep in mind, in which direction we fit, when using MarlinTrk)
   
   
   EVENT::TrackerHitVec::iterator it = trkHits.begin();
   
   //add hits to the MarlinTrk (in the same sorted order --> marlin_trk is from inside to the outside as well
   for( it = trkHits.begin() ; it != trkHits.end() ; ++it )
   {
      
      marlin_trk->addHit(*it);
      
   }
   
   
   int init_status = marlin_trk->initialise( MarlinTrk::IMarlinTrack::backward ) ; 
   //Here we first needed the order. We initialize the track to be fitted from outside in
   // As our track is sorted from inside out, this means, that we go backwards 
   //(compared to the order the hits are stored in the track)
   
   
   if (init_status == 0){ //Initialisation worked
      
      int fit_status = marlin_trk->fit() ; //fit the track
      
      
      if( fit_status == 0 ){ //Fitting worked
    
         
         double chi2 = 0.;
         int Ndf = 0;
         const gear::Vector3D IP(0.,0.,0.); // nominal IP
         
         
         // propagate to the IP
         TrackStateImpl* trkState = new TrackStateImpl() ;

	 trkState->setLocation( TrackState::AtIP ) ;
	 
         int propagate_status = marlin_trk->propagate(IP, *trkState, chi2, Ndf ) ;
         
         
         if ( propagate_status == 0 ) { //Propagation worked
            
          
            // Set the track state, chi2 and Ndf
            _lcioTrack->addTrackState(trkState);
            _lcioTrack->setChi2(chi2) ;
            _lcioTrack->setNdf(Ndf) ;
            
            
            _chi2Prob = ROOT::Math::chisquared_cdf_c( chi2 , Ndf );
            
            
            
            
         }
         
      }
    
   }

   delete marlin_trk;
  
 
   
   
}


double FTDTrack::getQI() const{
  
   
   double QI = _chi2Prob * _hits.size() / 14.; // 14 for the largest possible number of hits on a FTDTrack. TODO: shouldn't be hardcoded
   
   // make sure QI is between 0 and 1
   if (QI > 1. ) QI = 1.;
   if (QI < 0. ) QI = 0.;
   
   return QI;
   
}






