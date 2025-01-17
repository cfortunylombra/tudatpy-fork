/*    Copyright (c) 2010-2019, Delft University of Technology
 *    All rights reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */

#include "expose_shape_setup.h"

#include "tudatpy/docstrings.h"
#include <tudat/simulation/environment_setup.h>
#include <tudat/astro/reference_frames/referenceFrameTransformations.h>

//#include <pybind11/chrono.h>
#include <pybind11/eigen.h>
#include <pybind11/functional.h>
//#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>

namespace py = pybind11;
namespace tss = tudat::simulation_setup;

namespace tudatpy {
namespace numerical_simulation {
namespace environment_setup {
namespace shape {

    void expose_shape_setup(py::module &m) {

        py::class_<tss::BodyShapeSettings,
                std::shared_ptr<tss::BodyShapeSettings>>(m, "BodyShapeSettings",
                                                         get_docstring("BodyShapeSettings").c_str());

        py::class_<tss::SphericalBodyShapeSettings,
                std::shared_ptr<tss::SphericalBodyShapeSettings>,
                tss::BodyShapeSettings>(m, "SphericalBodyShapeSettings",
                                        get_docstring("SphericalBodyShapeSettings").c_str())
                .def_property("radius", &tss::SphericalBodyShapeSettings::getRadius,
                              &tss::SphericalBodyShapeSettings::resetRadius,
                              get_docstring("SphericalBodyShapeSettings.radius").c_str());


        py::class_<tss::OblateSphericalBodyShapeSettings,
                std::shared_ptr<tss::OblateSphericalBodyShapeSettings>,
                tss::BodyShapeSettings>(m, "OblateSphericalBodyShapeSettings",
                                        get_docstring("OblateSphericalBodyShapeSettings").c_str())
                .def_property("equatorial_radius", &tss::OblateSphericalBodyShapeSettings::getEquatorialRadius,
                              &tss::OblateSphericalBodyShapeSettings::resetEquatorialRadius,
                              get_docstring("OblateSphericalBodyShapeSettings.equatorial_radius").c_str())
                .def_property("flattening", &tss::OblateSphericalBodyShapeSettings::getFlattening,
                              &tss::OblateSphericalBodyShapeSettings::resetFlattening,
                              get_docstring("OblateSphericalBodyShapeSettings.flattening").c_str());


        m.def("spherical",
              &tss::sphericalBodyShapeSettings,
              py::arg("radius"),
              get_docstring("spherical").c_str());

        m.def("spherical_spice",
              &tss::fromSpiceSphericalBodyShapeSettings,
              get_docstring("spherical_spice").c_str());

        m.def("oblate_spherical",
              &tss::oblateSphericalBodyShapeSettings,
              py::arg("equatorial_radius"),
              py::arg("flattening"),
              get_docstring("oblate_spherical").c_str());

    }

}// namespace shape
}// namespace environment_setup
}// namespace numerical_simulation
}// namespace tudatpy
