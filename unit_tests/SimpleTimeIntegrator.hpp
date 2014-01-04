/** @file
 * This file is part of SeisSol.
 *
 * @author Alexander Breuer (breuera AT in.tum.de, http://www5.in.tum.de/wiki/index.php/Dipl.-Math._Alexander_Breuer) *
 * @section LICENSE
 * This software was developed at Technische Universitaet Muenchen, who is the owner of the software.
 *
 * According to good scientific practice, publications on results achieved in whole or in part due to this software should cite at least one paper or referring to an URL presenting this software.
 *
 * The owner wishes to make the software available to all users to use, reproduce, modify, distribute and redistribute also for commercial purposes under the following conditions of the original BSD license. Linking this software module statically or dynamically with other modules is making a combined work based on this software. Thus, the terms and conditions of this license cover the whole combination. As a special exception, the copyright holders of this software give you permission to link it with independent modules or to instantiate templates and macros from this software's source files to produce an executable, regardless of the license terms of these independent modules, and to copy and distribute the resulting executable under terms of your choice, provided that you also meet, for each linked independent module, the terms and conditions of this license of that module.
 *
 * Copyright (c) 2014
 * Technische Universitaet Muenchen
 * Department of Informatics
 * Chair of Scientific Computing
 * http://www5.in.tum.de/
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * All advertising materials mentioning features or use of this software must display the following acknowledgement: This product includes software developed by the Technische Universitaet Muenchen (TUM), Germany, and its contributors.
 * Neither the name of the Technische Universitaet Muenchen, Munich, Germany nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @section DESCRIPTION
 * Simple time integrator.
 **/

#ifndef SIMPLETIMEINTEGRATOR_HPP_
#define SIMPLETIMEINTEGRATOR_HPP_

#include <Initializer/typedefs.hpp>

#include "DenseMatrix.hpp"

namespace unit_test {
  class SimpleTimeIntegrator;
};

/**
 * Simple implementation of the ADER-time integration in SeisSol.
 **/
class unit_test::SimpleTimeIntegrator {
  //private:
    //! transposed stiffness matrices (multiplied by the inverse of the mass matrix): \f$ M^{-1} K^\xi, M^{-1} K^\eta, M^{-1} K^\zeta \f$
    real m_transposedStiffnessMatrices[3][NUMBEROFBASISFUNCTIONS*NUMBEROFBASISFUNCTIONS];

    //! dense matrix functionality
    unit_test::DenseMatrix m_denseMatrix;
  public:
    /**
     * Initializes the time integrator with the given file.
     *
     * @param i_matricesPath path to matrices XML-file.
     **/
    void initialize( std::string &i_matricesPath ) {
      m_denseMatrix.readMatrices( i_matricesPath );

      // iterate over the three coordinates \f$ \xi, \eta, \zeta \f$
      for( unsigned int l_coordinate = 0; l_coordinate < 3; l_coordinate++ ) { 
        // initialize the transposed stiffness matrix (multiplied by the inverse of the mass matrix)
        m_denseMatrix.initializeMatrix( 56+l_coordinate,
                                        NUMBEROFBASISFUNCTIONS,
                                        NUMBEROFBASISFUNCTIONS,
                                        m_transposedStiffnessMatrices[l_coordinate] );
      }
    }

    /** 
     * Computes the time derivatives of the given unknowns.
     *
     * @param i_unknowns unknowns for which the time derivatives are computed.
     * @param i_aStar star matrix \f$ A^*_k \f$
     * @param i_bStar star matrix \f$ B^*_k \f$
     * @param i_cStar star matrix \f$ C^*_k \f$
     * @param o_timeDerivatives time derivates.
     **/
    void computeTimeDerivation( const real i_unknowns[NUMBEROFUNKNOWNS],
                                const real i_aStar[NUMBEROFVARIABLES*NUMBEROFVARIABLES],
                                const real i_bStar[NUMBEROFVARIABLES*NUMBEROFVARIABLES],
                                const real i_cStar[NUMBEROFVARIABLES*NUMBEROFVARIABLES],
                                      real o_timeDerivatives[ORDEROFTAYLORSERIESEXPANSION][NUMBEROFUNKNOWNS] ) {
      // temporary matrix for two-step multiplication
      real l_temporaryProduct[NUMBEROFUNKNOWNS];
     
      // copy unknowns to zeroth derivative
      for( int l_entry = 0; l_entry < NUMBEROFUNKNOWNS; l_entry++ ) { 
        o_timeDerivatives[0][l_entry] = i_unknowns[l_entry];
      }   

      // iterate over order in time
      for( int l_order = 1; l_order < ORDEROFTAYLORSERIESEXPANSION; l_order++ ) { 
        // set time derivatives of this order to zero
        std::fill( o_timeDerivatives[l_order], o_timeDerivatives[l_order+1], 0); 

        /*
         * reference coordinate: \f$ \xi \f$
         */
        // set temporary product to zero
        std::fill( l_temporaryProduct, l_temporaryProduct+NUMBEROFUNKNOWNS, 0); 

        // stiffness matrix multiplication
        m_denseMatrix.executeStandardMultiplication( NUMBEROFBASISFUNCTIONS, NUMBEROFVARIABLES, NUMBEROFBASISFUNCTIONS,
                                                     m_transposedStiffnessMatrices[0], o_timeDerivatives[l_order-1], l_temporaryProduct ); 

        // star matrix multiplcation
        m_denseMatrix.executeStandardMultiplication( NUMBEROFBASISFUNCTIONS, NUMBEROFVARIABLES, NUMBEROFVARIABLES,
                                                     l_temporaryProduct, i_aStar, o_timeDerivatives[l_order] );                                      
        /*
         * reference coordinate: \f$ \eta \f$
         */
        // set temporary product to zero
        std::fill( l_temporaryProduct, l_temporaryProduct+NUMBEROFUNKNOWNS, 0); 
        
        // stiffness matrix multiplication
        m_denseMatrix.executeStandardMultiplication( NUMBEROFBASISFUNCTIONS, NUMBEROFVARIABLES, NUMBEROFBASISFUNCTIONS,
                                                     m_transposedStiffnessMatrices[1], o_timeDerivatives[l_order-1], l_temporaryProduct ); 

        // star matrix multiplcation
        m_denseMatrix.executeStandardMultiplication( NUMBEROFBASISFUNCTIONS, NUMBEROFVARIABLES, NUMBEROFVARIABLES,
                                                     l_temporaryProduct, i_bStar, o_timeDerivatives[l_order] );                                      
        /*
         * reference coordinate: \f$ \zeta \f$
         */
        // set temporary product to zero
        std::fill( l_temporaryProduct, l_temporaryProduct+NUMBEROFUNKNOWNS, 0); 
        
        // stiffness matrix multiplication
        m_denseMatrix.executeStandardMultiplication( NUMBEROFBASISFUNCTIONS, NUMBEROFVARIABLES, NUMBEROFBASISFUNCTIONS,
                                                     m_transposedStiffnessMatrices[2], o_timeDerivatives[l_order-1], l_temporaryProduct ); 

        // star matrix multiplcation
        m_denseMatrix.executeStandardMultiplication( NUMBEROFBASISFUNCTIONS, NUMBEROFVARIABLES, NUMBEROFVARIABLES,
                                                     l_temporaryProduct, i_cStar, o_timeDerivatives[l_order] );                                      
      } 
    }

    /**
     * Computes the time integrals of the given unknowns.
     *
     * @param i_timeDerivatives time derivatives of the unknowns in the cell.
     * @param i_deltaT integration boundaries \f$ \Delta t^\text{lo} \f$ and \f$ \Delta t^\text{up} \f$ relative to the time level \f$ \t^\text{cell} \f$ of the cell; integration is performned over the interval \f$ [ t^\text{cell} + \Delta t^\text{lo}, t^\text{cell} + \Delta t^\text{up} ] \f$
     * @param o_timeIntegratedUnknowns unknowns integrated over the specified interval.
     **/
    void computeTimeIntegration( const real i_timeDerivatives[ORDEROFTAYLORSERIESEXPANSION][NUMBEROFUNKNOWNS],
                                 const real i_deltaT[2],
                                       real o_timeIntegratedUnknowns[NUMBEROFUNKNOWNS] ) {
      // assert integration forward in time
      TS_ASSERT( i_deltaT[1] > i_deltaT[0] );

      // initialization of Taylor series factors
      real l_taylorSeriesFactors[2];
      l_taylorSeriesFactors[0] = i_deltaT[0];
      l_taylorSeriesFactors[1] = i_deltaT[1];
      real l_taylorSeriesFactor = l_taylorSeriesFactors[1] - l_taylorSeriesFactors[0];

      // update time integrated unknowns with zeroth derivatives
      for( int l_entry = 0; l_entry < NUMBEROFUNKNOWNS; l_entry++ ) {
        o_timeIntegratedUnknowns[l_entry] = l_taylorSeriesFactor * i_timeDerivatives[0][l_entry];
      }

      // iterate over order in time
      for( int l_order = 1; l_order < ORDEROFTAYLORSERIESEXPANSION; l_order++ ) {
        // compute factor of the taylor series
        l_taylorSeriesFactors[0] = -l_taylorSeriesFactors[0] * i_deltaT[0] / real(l_order+1);
        l_taylorSeriesFactors[1] = -l_taylorSeriesFactors[1] * i_deltaT[1] / real(l_order+1);
        l_taylorSeriesFactor = l_taylorSeriesFactors[1] - l_taylorSeriesFactors[0];
       
        // update time integrated unknowns
        for( int l_entry = 0; l_entry < NUMBEROFUNKNOWNS; l_entry++ ) {
          o_timeIntegratedUnknowns[l_entry] += l_taylorSeriesFactor * i_timeDerivatives[l_order][l_entry];
        } 
      }
    }
    
    /**
     * Computes the time integrals of the given unknowns.
     *
     * @param i_timeDerivatives time derivatives of the unknowns in the cell.
     * @param i_deltaT integration boundaries \f$ \Delta t^\text{lo} \f$ and \f$ \Delta t^\text{up} \f$ relative to the time level \f$ \t^\text{cell} \f$ of the cell; integration is performned over the interval \f$ [ t^\text{cell} + \Delta t^\text{lo}, t^\text{cell} + \Delta t^\text{up} ] \f$
     * @param o_timeIntegratedUnknowns unknowns integrated over the specified interval.
     **/
    void computeTimeIntegration( const real* i_timeDerivatives,
                                 const real i_deltaT[2],
                                       real* o_timeIntegratedUnknowns ) {
      // assert integration forward in time
      TS_ASSERT( i_deltaT[1] > i_deltaT[0] );

      // initialization of Taylor series factors
      real l_taylorSeriesFactors[2];
      l_taylorSeriesFactors[0] = i_deltaT[0];
      l_taylorSeriesFactors[1] = i_deltaT[1];
      real l_taylorSeriesFactor = l_taylorSeriesFactors[1] - l_taylorSeriesFactors[0];

      // update time integrated unknowns with zeroth derivatives
      for( int l_entry = 0; l_entry < NUMBEROFUNKNOWNS; l_entry++ ) {
        o_timeIntegratedUnknowns[l_entry] = l_taylorSeriesFactor * i_timeDerivatives[l_entry];
      }

      // iterate over order in time
      for( int l_order = 1; l_order < ORDEROFTAYLORSERIESEXPANSION; l_order++ ) {
        // compute factor of the taylor series
        l_taylorSeriesFactors[0] = -l_taylorSeriesFactors[0] * i_deltaT[0] / real(l_order+1);
        l_taylorSeriesFactors[1] = -l_taylorSeriesFactors[1] * i_deltaT[1] / real(l_order+1);
        l_taylorSeriesFactor = l_taylorSeriesFactors[1] - l_taylorSeriesFactors[0];
       
        // update time integrated unknowns
        for( int l_entry = 0; l_entry < NUMBEROFUNKNOWNS; l_entry++ ) {
          o_timeIntegratedUnknowns[l_entry] += l_taylorSeriesFactor * i_timeDerivatives[l_entry+(l_order*NUMBEROFUNKNOWNS)];
        } 
      }
    }
};
#endif
