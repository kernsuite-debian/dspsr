/***************************************************************************
 *
 *   Copyright (C) 2006 by Willem van Straten
 *   Licensed under the Academic Free License version 2.1
 *
 ***************************************************************************/

using namespace std;

#include "dsp/SubByteTwoBitCorrection.h"
#include "dsp/TwoBitTable.h"

#include <assert.h>

template<class Mask>
void dsp::SubByteTwoBitCorrection::dig_unpack (Mask& mask,
					       float* output_data,
					       const unsigned char* input_data,
					       uint64 ndat,
					       unsigned digitizer,
					       unsigned* weights,
					       unsigned nweights)
{
  if (verbose)
    cerr << "dsp::SubByteTwoBitCorrection::dig_unpack" << endl;

  if (!values)
    throw Error (InvalidState, "dsp::SubByteTwoBitCorrection::dig_unpack",
		 "not built");

  const unsigned ndig = get_ndig_per_byte();
  const unsigned samples_per_byte = table->get_values_per_byte() / ndig;

  if (ndig < 2)
    throw Error (InvalidState, "dsp::SubByteTwoBitCorrection::dig_unpack",
		 "number of digitizers per byte = %d must be > 1", ndig);

  const unsigned input_incr = get_input_incr ();
  const unsigned output_incr = get_output_incr ();
  const unsigned ndat_per_weight = get_ndat_per_weight ();

  // although I and Q are switched here, the histogram is queried as expected
  unsigned long*  hist = 0;
  if (keep_histogram)
    hist = get_histogram (digitizer);

  unsigned required_nweights = (unsigned) ceil (float(ndat)/float(ndat_per_weight));

  if (weights)
  {
    if (verbose)
      cerr << "dsp::SubByteTwoBitCorrection::dig_unpack nweights=" 
	   << nweights << endl;

    if (required_nweights > nweights)
      throw Error (InvalidParam, "dsp::SubByteTwoBitCorrection::dig_unpack",
		   "weights array size=%d < nweights=%d",
		   nweights, required_nweights);

  }

  nweights = required_nweights;
  uint64 points_left = ndat;
  unsigned bad_weights = 0;

  uint64 points = ndat_per_weight;

  for (unsigned wt=0; wt<nweights; wt++)
  {
    if (points > points_left)
      points = points_left;

    uint64 pt = 0;

    // retrieve the next points values from the 2bit data
    while (pt < points)
    {
      for (unsigned isamp=0; isamp<samples_per_byte; isamp++)
      {
	values[pt] = mask.twobit (*input_data, isamp);
	pt++;
      }
      input_data += input_incr;
    }

    assert (pt <= values_size);

    // calculate the weight based on the last ndat_per_weight pts
    unsigned n_in = 0;
    for (pt=0; pt<ndat_per_weight; pt++)
      n_in += lovoltage [values[pt]];

    if (hist)
      hist [n_in] ++;

    // test if the number of low voltage states is outside the
    // acceptable limit or if this section of data has been previously
    // flagged bad (for example, due to bad data in the other polarization)
    if ( n_in<nlow_min || n_in>nlow_max || (weights && weights[wt]==0) ) {

#ifdef _DEBUG
      cerr << "w[" << wt << "]=0 ";
#endif

      if (weights)
        weights[wt] = 0;

      // reduce the risk of other functions accessing un-initialized 
      // segments of the array
      for (pt=0; pt<points; pt++)
      {
	*output_data = 0.0;
	output_data += output_incr;
      }

      bad_weights ++;

    }

    else
    {
      float* corrected = &(dls_lookup[0]) + (n_in-nlow_min) * 4;
      for (pt=0; pt<points; pt++)
      {
	*output_data = corrected [values[pt]];
	output_data += output_incr;
      }
      if (weights)
	weights[wt] += n_in;
    }

    points_left -= points;
  }

  if (verbose && bad_weights)
    cerr << "dsp::SubByteTwoBitCorrection::dig_unpack " << bad_weights
	 << "/" << nweights << " bad weights" << endl;

}

