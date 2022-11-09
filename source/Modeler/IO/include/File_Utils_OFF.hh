/*----------------------
Copyright (C): Henri Payno, Axel Delsol, 
Laboratoire de Physique de Clermont UMR 6533 CNRS-UCA

This software is distributed under the terms
of the GNU Lesser General  Public Licence (LGPL)
See LICENSE.md for further details
----------------------*/
#ifndef FILE_UTILS_OFF_HH
#define FILE_UTILS_OFF_HH

#include "Mesh3DSettings.hh"
#include "DiscoidalCell.hh"
#include "SpheroidalCell.hh"

#include <CGAL/IO/Color.h>
#include <CGAL/Polyhedron_3.h>

//////////////////////////////////////////////////////////////////////////////
/// @namespace IO
//////////////////////////////////////////////////////////////////////////////
namespace IO
{
	//////////////////////////////////////////////////////////////////////////////
	/// \brief describe file management of the Object file format
	/// @author Henri Payno
	//////////////////////////////////////////////////////////////////////////////
	namespace OFF
	{
		using namespace Settings::Geometry;	
		using namespace Settings::Geometry::Mesh3D;	
			
		/// \brief export a polyhedron with a .off format
		bool exportPolyhedronToOff(Polyhedron_3*, std::ofstream*, std::map<Point_3, unsigned long int, comparePoint_3>&, bool randomColor = true, CGAL::Color pColor = CGAL::WHITE );
		/// \brief export a spheroidal cell to the .off format.
		bool exportSpheroidalCellToOff(SpheroidalCell*, std::ofstream*, std::map<Point_3, unsigned long int, comparePoint_3>&, std::vector<Polyhedron_3*> = std::vector<Polyhedron_3*>() );
		/// \brief create an off File with the cpop header
		std::ofstream* createOffFileWithHeader(std::string pPath);
		/// \brief export the vertices to an off file
		bool exportVerticesToOff(std::set<Point_3, comparePoint_3>&, std::map<Point_3, unsigned long int, comparePoint_3>&, std::ofstream* );
		/// \brief export the vertices to an off file
		bool exportVerticesToOff(std::set<Point_2>&, std::map<Point_2, unsigned long int>&, std::ofstream* );

		bool exportVertices(std::map<Point_3, unsigned long int, comparePoint_3>&, std::ofstream* );
		/// \brief export a facet to the off format.
		bool exportFacetToOff(const Polyhedron_3::Facet*, std::ofstream*, std::map<Point_3, unsigned long int, comparePoint_3>&, QString = QString() );
	}
}

#endif // FILE_UTILS_OFF_HH