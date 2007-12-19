//-*-C++-*-
/***************************************************************************
 *
 *   Copyright (C) 2007 by Willem van Straten
 *   Licensed under the Academic Free License version 2.1
 *
 ***************************************************************************/

/* $Source: /cvsroot/dspsr/dspsr/Kernel/Classes/dsp/Digitizer.h,v $
   $Revision: 1.1 $
   $Date: 2007/12/18 11:54:11 $
   $Author: straten $ */


#ifndef __Digitizer_h
#define __Digitizer_h

#include "dsp/Transformation.h"
#include "dsp/TimeSeries.h"
#include "dsp/BitSeries.h"

namespace dsp {

  //! Convert floating point samples to N-bit samples
  class Digitizer : public Transformation <TimeSeries, BitSeries>
  {

  public:
    
    //! Constructor
    Digitizer (const char* name = "Digitizer");
    
    //! Copy the input attributes to the output
    virtual void prepare ();

    //! Resize the output
    virtual void resize_output ();

   protected:

    virtual void transformation ();
    
    //! Perform the digitization
    virtual void pack () = 0;

  };

}

#endif // !defined(__Digitizer_h)