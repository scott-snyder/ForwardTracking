#include "AutHit.h"


#include "UTIL/ILDConf.h"

using namespace FTrack;

AutHit::AutHit( TrackerHit* trackerHit ){
   
   
   _trackerHit = trackerHit;

   //Set the position of the AutHit
   const double* pos= trackerHit->getPosition();
   _x = pos[0];
   _y = pos[1]; 
   _z = pos[2]; 


   //find out layer, module, sensor

   UTIL::BitField64  cellID( ILDCellID0::encoder_string );

   cellID.setValue( trackerHit->getCellID1() );
      
   _side   = cellID[ ILDCellID0::side   ];
   _layer  = cellID[ ILDCellID0::layer  ] + 1;
   _module = cellID[ ILDCellID0::module ];
   _sensor = cellID[ ILDCellID0::sensor ];
 
   
   //We assume a real hit. If it is virtual, this has to be set.
   _isVirtual = false;
   
   
}