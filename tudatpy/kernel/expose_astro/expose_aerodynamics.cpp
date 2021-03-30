/*    Copyright (c) 2010-2018, Delft University of Technology
 *    All rights reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */


#include "expose_aerodynamics.h"

#include <tudat/astro/aerodynamics/aerodynamicAcceleration.h>
#include <tudat/astro/aerodynamics/aerodynamicCoefficientGenerator.h>
#include <tudat/astro/aerodynamics/aerodynamicCoefficientInterface.h>
#include <tudat/astro/aerodynamics/aerodynamicForce.h>
#include <tudat/astro/aerodynamics/aerodynamicGuidance.h>
#include <tudat/astro/aerodynamics/aerodynamics.h>
#include <tudat/astro/aerodynamics/aerodynamicTorque.h>
#include <tudat/astro/aerodynamics/atmosphereModel.h>
#include <tudat/astro/aerodynamics/controlSurfaceAerodynamicCoefficientInterface.h>
#include <tudat/astro/aerodynamics/customAerodynamicCoefficientInterface.h>
#include <tudat/astro/aerodynamics/customConstantTemperatureAtmosphere.h>
#include <tudat/astro/aerodynamics/equilibriumWallTemperature.h>
#include <tudat/astro/aerodynamics/exponentialAtmosphere.h>
#include <tudat/astro/aerodynamics/flightConditions.h>
#include <tudat/astro/aerodynamics/hypersonicLocalInclinationAnalysis.h>
//#include "aerodynamics/nrlmsise00Atmosphere.h"
//#include "aerodynamics/nrlmsise00InputFunctions.h"
#include <tudat/astro/aerodynamics/standardAtmosphere.h>
#include <tudat/astro/aerodynamics/tabulatedAtmosphere.h>
#include <tudat/astro/aerodynamics/trimOrientation.h>
#include <tudat/astro/aerodynamics/windModel.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>

namespace py = pybind11;

namespace ta = tudat::aerodynamics;
namespace tr = tudat::reference_frames;

namespace tudat
{

namespace aerodynamics
{

class PyAerodynamicGuidance : public ta::AerodynamicGuidance {
public:
    /* Inherit the constructors */
    using AerodynamicGuidance::AerodynamicGuidance;
    using AerodynamicGuidance::currentAngleOfAttack_;
    using AerodynamicGuidance::currentAngleOfSideslip_;
    using AerodynamicGuidance::currentBankAngle_;

    void updateGuidance( const double currentTime ) override {
        PYBIND11_OVERLOAD_PURE(void, AerodynamicGuidance, updateGuidance, currentTime ); }
};

double getTotalSurfaceArea( const std::shared_ptr< HypersonicLocalInclinationAnalysis > coefficientGenerator )
{
    double totalSurfaceArea = 0.0;
    for( int i = 0; i < coefficientGenerator->getNumberOfVehicleParts( ); i++ )
    {
        totalSurfaceArea += std::fabs( coefficientGenerator->getVehiclePart( i )->getTotalArea( ) );
    }
    return totalSurfaceArea;
}


//! Function that saves the vehicle mesh data used for a HypersonicLocalInclinationAnalysis to a file
std::pair< std::vector< Eigen::Vector3d >, std::vector< Eigen::Vector3d > > getVehicleMesh(
        const std::shared_ptr< HypersonicLocalInclinationAnalysis > localInclinationAnalysis )
{
    std::vector< boost::multi_array< Eigen::Vector3d, 2 > > meshPoints =
            localInclinationAnalysis->getMeshPoints( );
    std::vector< boost::multi_array< Eigen::Vector3d, 2 > > meshSurfaceNormals =
            localInclinationAnalysis->getPanelSurfaceNormals( );


//    boost::array< int, 3 > independentVariables;
//    independentVariables[ 0 ] = 0;
//    independentVariables[ 1 ] = 6;
//    independentVariables[ 2 ] = 0;

//    std::vector< std::vector< std::vector< double > > > pressureCoefficients =
//            localInclinationAnalysis->getPressureCoefficientList( independentVariables );

    int counter = 0;
    std::vector< Eigen::Vector3d > meshPointsList;
    std::vector< Eigen::Vector3d > surfaceNormalsList;
//    std::map< int, Eigen::Vector1d > pressureCoefficientsList;

    for( unsigned int i = 0; i < meshPoints.size( ); i++ )
    {
        for( unsigned int j = 0; j < meshPoints.at( i ).shape( )[ 0 ] - 1; j++ )
        {
            for( unsigned int k = 0; k < meshPoints.at( i ).shape( )[ 1 ] - 1; k++ )
            {
                meshPointsList.push_back( meshPoints[ i ][ j ][ k ] );
                surfaceNormalsList.push_back( meshSurfaceNormals[ i ][ j ][ k ] );
//                pressureCoefficientsList[ counter ] = ( Eigen::Vector1d( ) << pressureCoefficients[ i ][ j ][ k ] ).finished( );
                counter++;
            }
        }
    }

    return std::make_pair( meshPointsList, surfaceNormalsList );
}

}

}

namespace tudatpy {

void expose_aerodynamics(py::module &m) {

    py::enum_<ta::AerodynamicCoefficientsIndependentVariables>(m, "AerodynamicCoefficientsIndependentVariables", "<no_doc>")
            .value("mach_number_dependent", ta::AerodynamicCoefficientsIndependentVariables::mach_number_dependent)
            .value("angle_of_attack_dependent", ta::AerodynamicCoefficientsIndependentVariables::angle_of_attack_dependent)
            .value("sideslip_angle_dependent", ta::AerodynamicCoefficientsIndependentVariables::angle_of_sideslip_dependent)
            .value("altitude_dependent", ta::AerodynamicCoefficientsIndependentVariables::altitude_dependent)
            .value("time_dependent", ta::AerodynamicCoefficientsIndependentVariables::time_dependent)
            .value("control_surface_deflection_dependent", ta::AerodynamicCoefficientsIndependentVariables::control_surface_deflection_dependent)
            .value("undefined_independent_variable", ta::AerodynamicCoefficientsIndependentVariables::undefined_independent_variable)
            .export_values();

    py::class_<ta::AerodynamicCoefficientInterface,
            std::shared_ptr<ta::AerodynamicCoefficientInterface>>(m, "AerodynamicCoefficientInterface" )
            .def_property_readonly("reference_area", &ta::AerodynamicCoefficientInterface::getReferenceArea )
            .def_property_readonly("current_force_coefficients", &ta::AerodynamicCoefficientInterface::getCurrentForceCoefficients )
            .def_property_readonly("current_moment_coefficients", &ta::AerodynamicCoefficientInterface::getCurrentMomentCoefficients )
            .def_property_readonly("current_coefficients", &ta::AerodynamicCoefficientInterface::getCurrentAerodynamicCoefficients )
            .def("update_coefficients", &ta::AerodynamicCoefficientInterface::updateCurrentCoefficients,
                 py::arg( "independent_variables" ),
                 py::arg( "time") );



    py::class_<ta::AerodynamicCoefficientGenerator<3, 6>,
            std::shared_ptr<ta::AerodynamicCoefficientGenerator<3, 6>>,
            ta::AerodynamicCoefficientInterface>(m, "AerodynamicCoefficientGenerator36", "<no_doc, only_dec>");

    m.def("get_default_local_inclination_mach_points", &ta::getDefaultHypersonicLocalInclinationMachPoints,
          py::arg( "mach_regime" ) = "Full" );


    m.def("get_default_local_inclination_angle_of_attack_points", &ta::getDefaultHypersonicLocalInclinationAngleOfAttackPoints );

    m.def("get_default_local_inclination_sideslip_angle_points", &ta::getDefaultHypersonicLocalInclinationAngleOfSideslipPoints );

    py::class_<ta::HypersonicLocalInclinationAnalysis,
            std::shared_ptr<ta::HypersonicLocalInclinationAnalysis>,
            ta::AerodynamicCoefficientGenerator<3, 6>>(m, "HypersonicLocalInclinationAnalysis" )
            .def(py::init<
                 const std::vector< std::vector< double > >&,
                 const std::shared_ptr< tudat::SurfaceGeometry >,
                 const std::vector< int >&,
                 const std::vector< int >&,
                 const std::vector< bool >&,
                 const std::vector< std::vector< int > >&,
                 const double,
                 const double,
                 const Eigen::Vector3d&,
                 const bool >(),
                 py::arg("independent_variable_points"),
                 py::arg("body_shape"),
                 py::arg("number_of_lines"),
                 py::arg("number_of_points"),
                 py::arg("invert_orders"),
                 py::arg("selected_methods"),
                 py::arg("reference_area"),
                 py::arg("reference_length"),
                 py::arg("moment_reference_point"),
                 py::arg("save_pressure_coefficients") = false );

    m.def("get_local_inclination_total_vehicle_area", &ta::getTotalSurfaceArea,
          py::arg( "local_inclination_analysis_object" ) );

    m.def("get_local_inclination_mesh", &ta::getVehicleMesh,
            py::arg( "local_inclination_analysis_object" ) );


    py::class_<ta::FlightConditions,
            std::shared_ptr<ta::FlightConditions>>(m, "FlightConditions")
            .def(py::init<
                 const std::shared_ptr<tudat::basic_astrodynamics::BodyShapeModel>,
                 const std::shared_ptr<tudat::reference_frames::AerodynamicAngleCalculator>>(),
                 py::arg("shape_model"),
                 py::arg("aerodynamic_angle_calculator") = std::shared_ptr< tr::AerodynamicAngleCalculator>())
            .def("get_aerodynamic_angle_calculator", &ta::FlightConditions::getAerodynamicAngleCalculator)
            .def("update_conditions", &ta::FlightConditions::updateConditions, py::arg("current_time") )
            .def_property_readonly("aerodynamic_angle_calculator", &ta::FlightConditions::getAerodynamicAngleCalculator)
            .def_property_readonly("current_altitude", &ta::FlightConditions::getCurrentAltitude)
            .def_property_readonly("current_longitude", &ta::FlightConditions::getCurrentLongitude)
            .def_property_readonly("current_geodetic_latitude", &ta::FlightConditions::getCurrentTime)
            .def_property_readonly("current_time", &ta::FlightConditions::getCurrentGeodeticLatitude)
            .def_property_readonly("current_body_centered_body_fixed_state", &ta::FlightConditions::getCurrentBodyCenteredBodyFixedState)
            .def_property_readonly("current_altitude", &ta::FlightConditions::getCurrentAltitude);

    py::class_<ta::AtmosphericFlightConditions,
            std::shared_ptr<ta::AtmosphericFlightConditions>,
            ta::FlightConditions>(m, "AtmosphericFlightConditions")
            .def_property_readonly("current_density", &ta::AtmosphericFlightConditions::getCurrentDensity)
            .def_property_readonly("current_temperature", &ta::AtmosphericFlightConditions::getCurrentFreestreamTemperature)
            .def_property_readonly("current_dynamic_pressure", &ta::AtmosphericFlightConditions::getCurrentDynamicPressure)
            .def_property_readonly("current_pressure", &ta::AtmosphericFlightConditions::getCurrentPressure)
            .def_property_readonly("current_airspeed", &ta::AtmosphericFlightConditions::getCurrentAirspeed)
            .def_property_readonly("current_mach_number", &ta::AtmosphericFlightConditions::getCurrentMachNumber)
            .def_property_readonly("current_airspeed_velocity", &ta::AtmosphericFlightConditions::getCurrentAirspeedBasedVelocity)
            .def_property_readonly("current_speed_of_sound", &ta::AtmosphericFlightConditions::getCurrentSpeedOfSound)
            .def_property_readonly("current_aerodynamic_coefficient_independent_variables",
                                   &ta::AtmosphericFlightConditions::getAerodynamicCoefficientIndependentVariables)
            .def_property_readonly("current_control_surface+aerodynamic_coefficient_independent_variables",
                                   &ta::AtmosphericFlightConditions::getControlSurfaceAerodynamicCoefficientIndependentVariables)
            .def_property_readonly("aerodynamic_coefficient_interface", &ta::AtmosphericFlightConditions::getAerodynamicCoefficientInterface);


    py::class_<ta::AerodynamicGuidance, ta::PyAerodynamicGuidance,
            std::shared_ptr< ta::AerodynamicGuidance > >(m, "AerodynamicGuidance")
            .def(py::init<>())
            .def("updateGuidance", &ta::AerodynamicGuidance::updateGuidance, py::arg("current_time") )
            .def_readwrite("angle_of_attack", &ta::PyAerodynamicGuidance::currentAngleOfAttack_)
            .def_readwrite("bank_angle", &ta::PyAerodynamicGuidance::currentBankAngle_)
            .def_readwrite("sideslip_angle", &ta::PyAerodynamicGuidance::currentAngleOfSideslip_);

};

};// namespace tudatpy
