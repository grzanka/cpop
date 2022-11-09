/*----------------------
Copyright (C): Henri Payno, Axel Delsol,
Laboratoire de Physique de Clermont UMR 6533 CNRS-UCA

This software is distributed under the terms
of the GNU Lesser General  Public Licence (LGPL)
See LICENSE.md for further details
----------------------*/
#include<iostream>
#include<stdio.h>

#include "SpheroidalCellMesh.hh"

#include "CellMeshSettings.hh"
#include "EngineSettings.hh"
#include "Geometry_Utils_Sphere.hh"
#include "File_Utils_OFF.hh"
#include "File_Utils_TXT.hh"
#include "Round_Shape.hh"
#include "SpheroidalCell_MeshSub_Thread.hh"
#include "UnitSystemManager.hh"

#include <CGAL/convex_hull_3.h>

#ifdef WITH_GDML_EXPORT
	#include "MyGDML_Parser.hh"	// The GDML parser.
#endif

#ifdef G4_LINK
	#include "G4Material.hh"

	#ifdef WITH_GEANT_4	// if using directly G4
		#include "G4VPhysicalVolume.hh"
		#include "G4LogicalVolume.hh"
		#include "G4Box.hh"
		#include "G4TessellatedSolid.hh"
	#else			// else using our own G4 definition from CAD Mesh
		#include "geometry/management/G4VPhysicalVolume.hh"
		#include "geometry/management/G4LogicalVolume.hh"
		#include "geometry/solid/specific/G4TessellatedSolid.hh"
		#include "geometry/solid/G4Box.hh"
	#endif // WITH_GEANT 4

#endif // G4_LINK

using namespace std;

#ifndef NDEBUG
	#define SPHEROIDAL_CELL_MESH_DEBUG 1
#else
	#define SPHEROIDAL_CELL_MESH_DEBUG 0	// must always stay at 0
#endif

using namespace CLHEP;

using namespace std;


//////////////////////////////////////////////////////////////////////////////////////////////
/// \param nbFacetPerCell 	The maximal number of facet a cell must contained
/// \param delta 			The minimal value for which we continu to reffine
/// \param pCells 			The set of cell to include on the mesh.
//////////////////////////////////////////////////////////////////////////////////////////////
SpheroidalCellMesh::SpheroidalCellMesh(	unsigned int nbFacetPerCell,
										double delta,
										set<t_Cell_3* > pCells) :
	Voronoi_3D_Mesh(nbFacetPerCell, delta, pCells),
	CellMesh()
{

}

//////////////////////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////////////////////
SpheroidalCellMesh::~SpheroidalCellMesh()
{
	clean();
}

//////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////
void SpheroidalCellMesh::clean()
{
	Voronoi_3D_Mesh::clean();
}

//////////////////////////////////////////////////////////////////////////////
/// \param pPath The output path file
/// \param pFormat The requested file format
/// \param pDivided True if we want to create a file for each cell
/// \return return values :
///					- 0 : success
///					- 1 : not implemented yet
///					- 2 : failed during export
///					- 3 : unknow format
//////////////////////////////////////////////////////////////////////////////
int SpheroidalCellMesh::exportToFile(QString pPath, MeshOutFormats::outputFormat pFormat, bool pDivided)
{
	assert(delaunay.is_valid());
	// if unknow format : return
	if(pFormat >= MeshOutFormats::Unknow )
	{
		return 3;
	}

	/// create cell
	vector<SpheroidalCell*> cells = generateMesh();



	int error = 0;

	switch(pFormat)
	{
		case MeshOutFormats::GDML:
		{
#ifdef WITH_GDML_EXPORT
			error = exportToFileGDML(pPath, cells, pDivided);
#else
			error = 1;
			InformationSystemManager::getInstance()->Message(InformationSystemManager::CANT_PROCESS_MES,
				"This option is unvailable for now. If you want to access it set option GDML_EXPORT to ON (cmake -DGDML_EXPORT=ON)",
				"SpheroidalCellMesh");
#endif
			break;
		}
		case MeshOutFormats::OFF:
		{
			error = exportToFileOff(pPath, cells, pDivided);
			break;
		}
		default:
		{
			InformationSystemManager::getInstance()->Message(InformationSystemManager::CANT_PROCESS_MES,
				"Exporter do not deal with this kind of format yet",
				"SpheroidalCellMesh");
			error = 1;
		}
	}

	return error;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \return the vector of SpheroidalCell containg a mesh generated by this function
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vector<SpheroidalCell*> SpheroidalCellMesh::generateMesh()
{
	neighboursCell.clear();
	assert( delaunay.is_valid() );

	// remove conflicting cells ( if one is included into an other one )
	removeConflicts();
	QString mess = "generateMesh "  + QString::number(delaunay.number_of_vertices() ) + " Cell(s) ";
        //InformationSystemManager::getInstance()->Message(InformationSystemManager::DEBUG_MES, mess.toStdString(), "SpheroidalCellMesh");

	vector<SpheroidalCell*> cells = getCellsStructure();

	map<SpheroidalCell*, set<const SpheroidalCell*> > neighbours = static_cast<const map<SpheroidalCell*, set<const SpheroidalCell*> > > (neighboursCell );

	if(!USE_THREAD_FOR_MESH_SUBDVN)
	{
		SpheroidalCellMeshSubThread reffinement(
												0,
												getMaxNbFacetPerCell(),
												getDeltaWin(),
												&neighbours,
												MAX_RATIO_NUCLEUS_TO_CELL
												);
		for(vector<SpheroidalCell*>::iterator itCell = cells.begin(); itCell != cells.end(); ++itCell)
		{
			reffinement.reffineCell(*itCell);
		}
	}else
	{
		vector<SpheroidalCellMeshSubThread*> reffinementThreads;
		// create the reffinement threads
		unsigned int nbThreadToCreate = (cells.size() / MIN_NB_CELL_PER_THREAD) + 1 ;
		{
			// create as much thread as needed
			if(nbThreadToCreate > INITIAL_MAX_THREAD)
			{
				nbThreadToCreate = INITIAL_MAX_THREAD;
			}

			unsigned int threadID = 0;
			while(reffinementThreads.size() < nbThreadToCreate)
			{
				if(SPHEROIDAL_CELL_MESH_DEBUG)
				{
					mess = "create a new thread of ID "  + QString::number(threadID);
					InformationSystemManager::getInstance()->Message(InformationSystemManager::DEBUG_MES, mess.toStdString(), "SpheroidalCellMesh");
				}

				reffinementThreads.push_back(	new SpheroidalCellMeshSubThread(
																					threadID,
																					getMaxNbFacetPerCell(),
																					getDeltaWin(),
																					&neighbours,
																					MAX_RATIO_NUCLEUS_TO_CELL
																				));
				threadID++;
			}
		}

		// add cells to threads
		{
			vector<SpheroidalCell*>::iterator itCell;
			int iCell = 0;
			for(itCell = cells.begin(); itCell != cells.end(); ++itCell)
			{
				// G4cout << " \n itCell getID " << itCell->getID() << G4endl;
				reffinementThreads[iCell%nbThreadToCreate]->addCell(*itCell);
				iCell++;
			}
		}

		// run threads.
		vector<SpheroidalCellMeshSubThread*>::iterator itThread;
		for(itThread = reffinementThreads.begin(); itThread != reffinementThreads.end(); ++itThread)
		{
			(*itThread)->start(MESHING_THREAD_PRIORITY);
		}

		/// wait until all threads process
		for(itThread = reffinementThreads.begin(); itThread != reffinementThreads.end(); ++itThread)
		{
			(*itThread)->wait();
		}

		//delete reffinement thread
		for( itThread = reffinementThreads.begin(); itThread != reffinementThreads.end(); ++itThread)
		{
			delete *itThread;
			*itThread = NULL;
		}
	}


	return cells;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////:
/// \param pPath 	The output path file
/// \param cells 	The list of cell to export
/// \return 		int return values :
///					- 0 : success
///					- 1 : not implemented yet
///					- 2 : failed during export
///
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////:
int SpheroidalCellMesh::exportToFileOff_undivided(QString pPath, std::vector<SpheroidalCell*>* cells)
{
	unsigned long int nbFacets = 0;				// hte total number of facet on the file
	set<Point_3, comparePoint_3> points; 						// all the points included inside the mesh
	map<Point_3, unsigned long int, comparePoint_3> indexes;	// the points and their ids
	/// create .off file for the mesh
	pPath += ".off";
	ofstream* voronoiOut = IO::OFF::createOffFileWithHeader(pPath.toStdString());

	// deal with markup points
	std::map<set<Point_3>, pair<CGAL::Color, double> > boxes;
	{
		// convert all of them to boxes
		std::map<Point_3, pair<CGAL::Color, double> >::const_iterator itMarkPoint;
		for( itMarkPoint = markupPoints.begin(); itMarkPoint != markupPoints.end(); ++itMarkPoint)
		{
			std::set<Point_3> boxPoints = Utils::myCGAL::convertPointToBox(itMarkPoint->first, itMarkPoint->second.second);
			boxes.insert(make_pair(boxPoints, itMarkPoint->second) );
			points.insert( boxPoints.begin(), boxPoints.end());
			Polyhedron_3 polyMark;
			CGAL::convex_hull_3(boxPoints.begin(), boxPoints.end(), polyMark);
			nbFacets += Utils::myCGAL::getNumberOfFacets(&polyMark);
		}
	}

	std::vector<SpheroidalCell*>::iterator itCell;
	Polyhedron_3::Point_iterator itPolyPts;
	for(itCell = (*cells).begin(); itCell != (*cells).end(); ++itCell)
	{
		points.insert((*itCell)->shape_points_begin(), (*itCell)->shape_points_end());
		// add shape facets
		nbFacets += Utils::myCGAL::getNumberOfFacets((*itCell)->getShape());

		// add nucleus points
		std::vector<t_Nucleus_3*> lNuclei = (*itCell)->getNuclei();
		std::vector<t_Nucleus_3*>::iterator itNucleus;
		for(itNucleus = lNuclei.begin(); itNucleus != lNuclei.end(); ++itNucleus)
		{
			std::vector<Point_3> lCellNucleusPoints = (*itNucleus)->getShapePoints();
			points.insert(lCellNucleusPoints.begin(), lCellNucleusPoints.end());
			// add the nucleus facets :
			Polyhedron_3 polyNucleus;
			CGAL::convex_hull_3(lCellNucleusPoints.begin(), lCellNucleusPoints.end(), polyNucleus);
			nbFacets += Utils::myCGAL::getNumberOfFacets(&polyNucleus);
		}
	}

	*voronoiOut << distance(points.begin(), points.end()) << " " << nbFacets << " 0" << endl;
	/// \todo : generate a thread to write on it
	IO::OFF::exportVerticesToOff(points, indexes, voronoiOut);
	for(itCell = (*cells).begin(); itCell != (*cells).end(); ++itCell)
	{
	    IO::OFF::exportSpheroidalCellToOff(*itCell, voronoiOut, indexes);
	}

	// export organelles points
	{
		std::map<set<Point_3>, pair<CGAL::Color, double> >::const_iterator itMark;
		for( itMark = boxes.begin(); itMark != boxes.end(); ++itMark)
		{
			Polyhedron_3 polyMark;
			CGAL::convex_hull_3(itMark->first.begin(), itMark->first.end(), polyMark);
			IO::OFF::exportPolyhedronToOff(&polyMark, voronoiOut, indexes, false, itMark->second.first);
		}
	}
	voronoiOut->close();
	delete voronoiOut;
    return 0;
}

#if defined(WITH_GEANT_4) || defined(WITH_GDML_EXPORT)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \param checkOverLaps true if we want to check geometry overlap by the G4 process
/// \param pMaterialBetwwenCell The material we want to set to cytoplasm
/// \param pMapCells The map linking G4LogicalVolume and t_Cell_3
/// \param pMapNuclei The map linking G4LogicalVolume and t_Nucleu
/// \param pExportNuclei true if we want to export nuclei as well to G4s_3
/// \return The world created corresponding to the cell population
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
G4PVPlacement* SpheroidalCellMesh::convertToG4World(bool checkOverLaps, G4Material* pMaterialBetwwenCell, map<const G4LogicalVolume*, const t_Cell_3* > * pMapCells,
		map<const G4LogicalVolume*, const t_Nucleus_3*> * pMapNuclei, bool pExportNuclei )
{
	InformationSystemManager::getInstance()->Message(InformationSystemManager::INFORMATION_MES, "starting convertion to G4World ", "SpheroidalCellMesh");

	assert( delaunay.is_valid() );
	G4Material*  worldMaterial = pMaterialBetwwenCell;
	if(!worldMaterial)
	{
		worldMaterial = MaterialManager::getInstance()->getDefaultMaterial();
	}

	double convertionToG4 = UnitSystemManager::getInstance()->getConversionToG4();

	// get bounding volume
	K::Iso_cuboid_3 boundingBox = getBoundingBox();

	// as this is the world, it must be centered on the origin
	G4Box* solidWorld= new G4Box(	"sWorld",
									G4double( max(fabs(boundingBox.xmax()), fabs(boundingBox.xmin()))/2. )* convertionToG4,
									G4double( max(fabs(boundingBox.ymax()), fabs(boundingBox.ymin()))/2. )* convertionToG4,
									G4double( max(fabs(boundingBox.zmax()), fabs(boundingBox.zmin()))/2. )* convertionToG4
								);


	G4LogicalVolume* logicWorld= new G4LogicalVolume( solidWorld, worldMaterial, "LV_World", 0, 0, 0);

	G4PVPlacement* physiWorld = new G4PVPlacement(	G4Transform3D( HepRotation(), G4ThreeVector(0., 0., 0.) ),	// no rotation
						                            logicWorld,     							// its logical volume
													"PV_World",     							// its name
						                            0,              							// its mother  volume
						                            false,          							// no boolean operations
						                            0,											// copy number
						                            checkOverLaps); 							// surface overlaps

	// then create the G4Logical volume

	if( !convertToG4Logical( logicWorld, checkOverLaps, worldMaterial, pMapCells, pMapNuclei, pExportNuclei ))
	{
		delete physiWorld;
		physiWorld = NULL;
		G4cout<< "SpheroidalCellMesh Logical" << G4endl;
	}

	G4cout<< "End convert to G4World" << G4endl;

	return physiWorld;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////:
/// \param pCenter The center of the bounding box created
/// \param checkOverLaps true if we want to check geometry overlap by the G4 process
/// \param pMaterialBetweenCell The material we want to set to cytoplasm
/// \return return the bounding box including all cells ( G4Box)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////:
G4LogicalVolume* SpheroidalCellMesh::getG4BoundingLogicalVolume( G4ThreeVector& pCenter, G4Material* pMaterialBetweenCell)
{
	double convertionToG4 = UnitSystemManager::getInstance()->getConversionToG4();

	G4Material*  spheroidMaterial = pMaterialBetweenCell;
	if( !spheroidMaterial )
	{
		spheroidMaterial = MaterialManager::getInstance()->getDefaultMaterial();
	}
	assert(spheroidMaterial);

	// get bounding volume
	K::Iso_cuboid_3 boundingBox = getBoundingBox();
	Point_3 lCenter(
		(boundingBox.xmax()*convertionToG4 + boundingBox.xmin()*convertionToG4)/2.,
		(boundingBox.ymax()*convertionToG4 + boundingBox.ymin()*convertionToG4)/2.,
		(boundingBox.zmax()*convertionToG4 + boundingBox.zmin()*convertionToG4)/2.);

	pCenter = G4ThreeVector(lCenter.x(), lCenter.y(), lCenter.z() );

	G4Box* solidBB= new G4Box(	"Spheroid_bb",
								G4double( (boundingBox.xmax() - boundingBox.xmin())/2. )* convertionToG4,
								G4double( (boundingBox.ymax() - boundingBox.ymin())/2. )* convertionToG4,
								G4double( (boundingBox.zmax() - boundingBox.zmin())/2. )* convertionToG4
								);

	cout << "bounding box in G4 reference = " <<  (max(fabs(boundingBox.xmax()), fabs(boundingBox.xmin()))/2. )* convertionToG4 << ", "
		<< (max(fabs(boundingBox.ymax()), fabs(boundingBox.ymin()))/2. )* convertionToG4 << ", "
		<< (max(fabs(boundingBox.zmax()), fabs(boundingBox.zmin()))/2. )* convertionToG4 << endl;

	cout << "origin of the spheroid in G4 reference = " << lCenter << endl;

	return new G4LogicalVolume( solidBB, spheroidMaterial, "LV_Spheroid_bb", 0, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////:
/// \param parent the parent of this cell mesh on G4 hierarchy
/// \param checkOverLaps true if we want to check geometry overlap by the G4 process
/// \param pMaterialBetweenCell The material we want to set to cytoplasm
/// \param pMapCells The map linking G4LogicalVolume and t_Cell_3
/// \param pMapNuclei The map linking G4LogicalVolume and t_Nucleus_3
/// \param pExportNuclei true if we want to export nuclei as well to G4
/// \return return the cell under a G4 structure ( G4Tesselated solid )
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////:
G4LogicalVolume* SpheroidalCellMesh::convertToG4Logical( G4LogicalVolume* parent, bool checkOverlaps, G4Material* pMaterialBetweenCell,
	map<const G4LogicalVolume*, const t_Cell_3* > * pMapCells, map<const G4LogicalVolume*, const t_Nucleus_3*> * pMapNuclei, bool pExportNuclei)
{
	assert( delaunay.is_valid() );

	G4ThreeVector pCenter;

	G4LogicalVolume* logicBB = getG4BoundingLogicalVolume(pCenter, pMaterialBetweenCell);

	new G4PVPlacement(
		G4Transform3D( HepRotation(), pCenter ),	// no rotation
		logicBB,     								// its logical volume
		"PV_World",     							// its name
		parent,             						// its mother  volume
		false,          							// no boolean operations
		0,											// copy number
		checkOverlaps); 							// surface overlaps

	/// \todo : call this function inConvertToG4World
	assert(delaunay.is_valid());
	InformationSystemManager::getInstance()->Message(InformationSystemManager::INFORMATION_MES, "starting convertion to G4Logical ", "SpheroidalCellMesh");

	vector<SpheroidalCell*> cells = generateMesh();
	if(cells.size() < 1)
	{
		return 0;
	}


	vector<SpheroidalCell*>::iterator itCell;
	int iPoly = 0;
	// set indexes
	Polyhedron_3::Point_iterator itPolyPts;

	/// \brief generate the cell mesh
	unsigned int nbRemovedForG4 = 0;
	vector<pair<G4TessellatedSolid*, G4Orb*> > solids;
	for(itCell = cells.begin(); itCell != cells.end(); ++itCell)
	{
		QString polyName = cellNamePrefix + QString::number(iPoly);

		// because of dimension changement from CPOP to G4 and numerical precision we can be forced to remove some cells to ensure no recovrement.
		map<SpheroidalCell*, set<const SpheroidalCell*> > neighbours = static_cast<const map<SpheroidalCell*, set<const SpheroidalCell*> > > (neighboursCell );


		if( !(*itCell)->convertToG4Structure(logicBB, polyName, checkOverlaps, &neighboursCell, getMaxNbFacetPerCell(), getDeltaWin(), pMapCells, pMapNuclei, pExportNuclei ) )
		{
			cout << "\n polyname : " << (polyName.toStdString()).c_str() << endl;
			nbRemovedForG4 ++;
		}

		iPoly ++;
	}
	cout << "\n\n\n Real number of cells : " <<  cells.size() << "\n" << endl;
	return logicBB;
}

#endif	// WITH_GEANT_4

#ifdef WITH_GDML_EXPORT	/// if generate a connection with G4 by GDML ( including or not G4)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \param pPath 	The output path file
/// \param cells 	The set of cell to export
/// \param pDivided True if we want to generate a file per cell
/// \return return values :
///					- 0 : success
///					- 1 : not implemented yet
///					- 2 : failed during export
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int SpheroidalCellMesh::exportToFileGDML(QString pPath, vector<SpheroidalCell*> cells, bool pCheckOverLaps )
{
	if(cells.size() < 1)
	{
		return 0;
	}

	G4PVPlacement* physiWorld = convertToG4World(pCheckOverLaps );

	// export the G4 world
	MyGDML_Parser GDMLParser;
	QString path = pPath + ".gdml";
	
	std::remove((path.toLocal8Bit()).data());
	GDMLParser.write(path, physiWorld);

	cout << "\n\n\n GDML written " << endl;

	delete physiWorld;

	return 0;
}
#endif //WITH_GDML_EXPORT
