/*
  Copyright 2013 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE BoxTest

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <boost/filesystem/path.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperty.hpp>


using namespace Opm;

inline std::string prefix() {
    return boost::unit_test::framework::master_test_suite().argv[1];
}

inline EclipseState makeState(const std::string& fileName) {
    Parser parser;
    boost::filesystem::path boxFile(fileName);
    auto deck =  parser.parseFile(boxFile.string(), ParseContext());
    return EclipseState( deck, ParseContext() );
}

BOOST_AUTO_TEST_CASE( PERMX ) {
    EclipseState state = makeState( prefix() + "BOX/BOXTEST1" );
    const auto& permx = state.get3DProperties().getDoubleGridProperty( "PERMX" );
    const auto& permy = state.get3DProperties().getDoubleGridProperty( "PERMY" );
    const auto& permz = state.get3DProperties().getDoubleGridProperty( "PERMZ" );
    size_t i, j, k;
    const EclipseGrid& grid = state.getInputGrid();

    for (k = 0; k < grid.getNZ(); k++) {
        for (j = 0; j < grid.getNY(); j++) {
            for (i = 0; i < grid.getNX(); i++) {

                BOOST_CHECK_CLOSE( permx.iget( i, j, k ) * 0.25, permz.iget( i, j, k ), 0.001 );
                BOOST_CHECK_EQUAL( permx.iget( i, j, k ) * 2, permy.iget( i, j, k ) );

            }
        }
    }
}



BOOST_AUTO_TEST_CASE( PARSE_BOX_OK ) {
    EclipseState state = makeState( prefix() + "BOX/BOXTEST1" );
    const auto& satnum = state.get3DProperties().getIntGridProperty( "SATNUM" );
    {
        size_t i, j, k;
        const EclipseGrid& grid = state.getInputGrid();
        for (k = 0; k < grid.getNZ(); k++) {
            for (j = 0; j < grid.getNY(); j++) {
                for (i = 0; i < grid.getNX(); i++) {

                    size_t g = i + j * grid.getNX() + k * grid.getNX() * grid.getNY();
                    if (i <= 1 && j <= 1 && k <= 1)
                        BOOST_CHECK_EQUAL( satnum.iget( g ), 10 );
                    else
                        BOOST_CHECK_EQUAL( satnum.iget( g ), 2 );

                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE( PARSE_MULTIPLY_COPY ) {
    EclipseState state = makeState( prefix() + "BOX/BOXTEST1" );
    const auto& satnum = state.get3DProperties().getIntGridProperty( "SATNUM" );
    const auto& fipnum = state.get3DProperties().getIntGridProperty( "FIPNUM" );
    size_t i, j, k;
    const EclipseGrid& grid = state.getInputGrid();

    for (k = 0; k < grid.getNZ(); k++) {
        for (j = 0; j < grid.getNY(); j++) {
            for (i = 0; i < grid.getNX(); i++) {

                size_t g = i + j * grid.getNX() + k * grid.getNX() * grid.getNY();
                if (i <= 1 && j <= 1 && k <= 1)
                    BOOST_CHECK_EQUAL( 4 * satnum.iget( g ), fipnum.iget( g ) );
                else
                    BOOST_CHECK_EQUAL( 2 * satnum.iget( i, j, k ), fipnum.iget( i, j, k ) );

            }
        }
    }
}




BOOST_AUTO_TEST_CASE( KEYWORD_BOX_TOO_SMALL) {
    BOOST_CHECK_THROW( makeState(prefix() + "BOX/BOXTEST3") , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE( EQUALS ) {
    EclipseState state = makeState( prefix() + "BOX/BOXTEST1" );
    const auto& pvtnum = state.get3DProperties().getIntGridProperty( "PVTNUM" );
    const auto& eqlnum = state.get3DProperties().getIntGridProperty( "EQLNUM" );
    const auto& poro = state.get3DProperties().getDoubleGridProperty( "PORO" );
    size_t i, j, k;
    const EclipseGrid& grid = state.getInputGrid();

    for (k = 0; k < grid.getNZ(); k++) {
        for (j = 0; j < grid.getNY(); j++) {
            for (i = 0; i < grid.getNX(); i++) {

                BOOST_CHECK_EQUAL( pvtnum.iget( i, j, k ), k );
                BOOST_CHECK_EQUAL( eqlnum.iget( i, j, k ), 77 + 2 * k );
                BOOST_CHECK_EQUAL( poro.iget( i, j, k ), 0.25 );

            }
        }
    }
}


BOOST_AUTO_TEST_CASE( OPERATE ) {
    EclipseState state = makeState( prefix() + "BOX/BOXTEST1" );
    const auto& ntg = state.get3DProperties().getDoubleGridProperty( "NTG" );

    BOOST_CHECK_EQUAL( ntg.iget( 0,0,0) , 8.50 );  // MULTA
    BOOST_CHECK_EQUAL( ntg.iget( 0,5,0) , 5.00 );  // POLY

    BOOST_CHECK_EQUAL( ntg.iget( 0,0,1) , 4.0 );  // COPY
    BOOST_CHECK_EQUAL( ntg.iget( 0,5,1) , 4.0 );  // MINLIM

    BOOST_CHECK_EQUAL( ntg.iget( 0,0,2) , 2.0 );  // MAXLIM
}

