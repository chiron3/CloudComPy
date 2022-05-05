//##########################################################################
//#                                                                        #
//#                              CloudComPy                                #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; either version 3 of the License, or     #
//#  any later version.                                                    #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#  You should have received a copy of the GNU General Public License     #
//#  along with this program. If not, see <https://www.gnu.org/licenses/>. #
//#                                                                        #
//#          Copyright 2020-2021 Paul RASCLE www.openfields.fr             #
//#                                                                        #
//##########################################################################

#include "cloudComPy.hpp"

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "converters.hpp"
#include "colorsPy.hpp"
#include "ScalarFieldPy.hpp"
#include "ccGenericCloudPy.hpp"
#include "ccOctreePy.hpp"
#include "ccPointCloudPy.hpp"
#include "ccMeshPy.hpp"
#include "ccPrimitivesPy.hpp"
#include "ccPolylinePy.hpp"
#include "distanceComputationToolsPy.hpp"
#include "geometricalAnalysisToolsPy.hpp"
#include "registrationToolsPy.hpp"
#include "cloudSamplingToolsPy.hpp"
#include "ccFacetPy.hpp"
#include "NeighbourhoodPy.hpp"

#include "initCC.h"
#include "pyCC.h"
#include "PyScalarType.h"
#include <ccGLMatrix.h>
#include <ccHObject.h>
#include <ccPointCloud.h>
#include <ScalarField.h>
#include <ccNormalVectors.h>
#include <ccHObjectCaster.h>
#include <ccRasterGrid.h>
#include <ccClipBox.h>
#include <ccCommon.h>
#include <AutoSegmentationTools.h>
#include <ParallelSort.h>

#include <QString>
#include <vector>

#include "optdefines.h"

#include "pyccTrace.h"
#include "cloudComPy_DocStrings.hpp"

namespace bp = boost::python;
namespace bnp = boost::python::numpy;

char const* greet()
{
   return "hello, world, this is CloudCompare Python Interface: 'CloudComPy'";
}

void initCC_py()
{
    PyObject *cc_module;
    cc_module = PyImport_ImportModule("cloudComPy");
    const char* modulePath = PyUnicode_AsUTF8(PyModule_GetFilenameObject(cc_module));
    CCTRACE("modulePath: " <<  modulePath);
    initCC::init(modulePath);
}

void initCloudCompare_py()
{
    pyCC* pyCCInternals = initCloudCompare();
}

const char* getScalarType()
{
    //Get the scalar type used in cloudCompare under the form defined in Numpy: 'float32' or 'float64'
    return CC_NPY_FLOAT_STRING;
}

struct ICPres
{
    ccPointCloud* aligned;
    ccGLMatrix transMat;
    double finalScale;
    double finalRMS;
    unsigned finalPointCount;
};

ICPres ICP_py(  ccHObject* data,
                ccHObject* model,
                double minRMSDecrease,
                unsigned maxIterationCount,
                unsigned randomSamplingLimit,
                bool removeFarthestPoints,
                CCCoreLib::ICPRegistrationTools::CONVERGENCE_TYPE method,
                bool adjustScale,
                double finalOverlapRatio = 1.0,
                bool useDataSFAsWeights = false,
                bool useModelSFAsWeights = false,
                int transformationFilters = CCCoreLib::RegistrationTools::SKIP_NONE,
                int maxThreadCount = 0)
{
    ICPres a;
    ICP(data,
        model,
        a.transMat,
        a.finalScale,
        a.finalRMS,
        a.finalPointCount,
        minRMSDecrease,
        maxIterationCount,
        randomSamplingLimit,
        removeFarthestPoints,
        method,
        adjustScale,
        finalOverlapRatio,
        useDataSFAsWeights,
        useModelSFAsWeights,
        transformationFilters,
        maxThreadCount);
    a.aligned = dynamic_cast<ccPointCloud*>(data);
    return a;
}

bp::tuple importFilePy(const char* filename,
    CC_SHIFT_MODE mode = AUTO,
    double x = 0,
    double y = 0,
    double z = 0)
{
    std::vector<ccMesh*> meshes;
    std::vector<ccPointCloud*> clouds;
    std::vector<ccHObject*> entities = importFile(filename, mode, x, y, z);
    for( auto entity : entities)
    {
        ccMesh* mesh = ccHObjectCaster::ToMesh(entity);
        if (mesh)
        {
            meshes.push_back(mesh);
        }
        else
        {
            ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(entity);
            if (cloud)
            {
                clouds.push_back(cloud);
            }
        }
    }
    bp::tuple res = bp::make_tuple(meshes, clouds);
    return res;
}

ccPointCloud* loadPointCloudPy(
    const char* filename,
    CC_SHIFT_MODE mode = AUTO,
    int skip = 0,
    double x = 0,
    double y = 0,
    double z = 0)
{
    std::vector<ccMesh*> meshes;
    std::vector<ccPointCloud*> clouds;
    std::vector<ccHObject*> entities = importFile(filename, mode, x, y, z);
    for( auto entity : entities)
    {
        ccMesh* mesh = ccHObjectCaster::ToMesh(entity);
        if (mesh)
        {
            meshes.push_back(mesh);
        }
        else
        {
            ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(entity);
            if (cloud)
            {
                clouds.push_back(cloud);
            }
        }
    }
    if (clouds.size())
        return clouds.back();
    else
        return nullptr;
}


ccMesh* loadMeshPy(
    const char* filename,
    CC_SHIFT_MODE mode = AUTO,
    int skip = 0,
    double x = 0,
    double y = 0,
    double z = 0)
{
    std::vector<ccMesh*> meshes;
    std::vector<ccPointCloud*> clouds;
    std::vector<ccHObject*> entities = importFile(filename, mode, x, y, z);
    for( auto entity : entities)
    {
        ccMesh* mesh = ccHObjectCaster::ToMesh(entity);
        if (mesh)
        {
            meshes.push_back(mesh);
        }
        else
        {
            ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(entity);
            if (cloud)
            {
                clouds.push_back(cloud);
            }
        }
    }
    if (meshes.size())
        return meshes.back();
    else
        return nullptr;
}

void deleteEntity(ccHObject* entity)
{
    delete entity;
}

bp::tuple ExtractSlicesAndContours_py
    (
    std::vector<ccHObject*> entities,
    ccBBox bbox,
    ccGLMatrix bboxTrans = ccGLMatrix(),
    bool singleSliceMode = true,
    bool processRepeatX =false,
    bool processRepeatY =false,
    bool processRepeatZ =true,
    bool extractEnvelopes = false,
    PointCoordinateType maxEdgeLength = 0,
    int envelopeType = 0,
    bool extractLevelSet = false,
    double levelSetGridStep = 0,
    int levelSetMinVertCount= 0,
    PointCoordinateType gap = 0,
    bool multiPass = false,
    bool splitEnvelopes = false,
    bool projectOnBestFitPlane = false,
    bool generateRandomColors = false)
{
    std::vector<ccGenericPointCloud*> clouds;
    std::vector<ccGenericMesh*> meshes;
    for(auto obj: entities)
    {
        if (obj->isKindOf(CC_TYPES::MESH))
        {
            ccMesh* mesh = ccHObjectCaster::ToMesh(obj);
            meshes.push_back(mesh);
        }
        else if(obj->isKindOf(CC_TYPES::POINT_CLOUD))
        {
            ccGenericPointCloud* cloud = ccHObjectCaster::ToGenericPointCloud(obj);
            clouds.push_back(cloud);
        }
    }
    CCTRACE("clouds: " << clouds.size() << " meshes: " << meshes.size());
    ccClipBox clipBox;
    clipBox.set(bbox, bboxTrans);
    clipBox.enableGLTransformation(true);
    bool processDimensions[3] = {processRepeatX, processRepeatY, processRepeatZ};

    Envelope_Type val[3] = {LOWER, UPPER, FULL};
    if (envelopeType <0) envelopeType = 0;
    if (envelopeType >2) envelopeType = 2;
    Envelope_Type envelType = val[envelopeType];

    std::vector<ccHObject*> outputSlices;
    std::vector<ccPolyline*> outputEnvelopes;
    std::vector<ccPolyline*> levelSet;
    ExtractSlicesAndContoursClone(clouds, meshes, clipBox, singleSliceMode, processDimensions, outputSlices,
                             extractEnvelopes, maxEdgeLength, envelType, outputEnvelopes,
                             extractLevelSet, levelSetGridStep, levelSetMinVertCount, levelSet,
                             gap, multiPass, splitEnvelopes, projectOnBestFitPlane, false, generateRandomColors, nullptr);
    bp::tuple res = bp::make_tuple(outputSlices, outputEnvelopes, levelSet);
    return res;
}

struct ComponentIndexAndSize
{
    unsigned index;
    unsigned size;

    ComponentIndexAndSize(unsigned i, unsigned s) : index(i), size(s) {}

    static bool DescendingCompOperator(const ComponentIndexAndSize& a, const ComponentIndexAndSize& b)
    {
        return a.size > b.size;
    }
};


std::vector<ccPointCloud*> createComponentsClouds_(   ccGenericPointCloud* cloud,
                                                      CCCoreLib::ReferenceCloudContainer& components,
                                                      unsigned minPointsPerComponent,
                                                      bool randomColors,
                                                      bool sortBysize=true)
{
    CCTRACE("createComponentsClouds_ " << randomColors);
    std::vector<ccPointCloud*> resultClouds;
    if (!cloud || components.empty())
        return resultClouds;

    std::vector<ComponentIndexAndSize> sortedIndexes;
    std::vector<ComponentIndexAndSize>* _sortedIndexes = nullptr;
    if (sortBysize)
    {
        try
        {
            sortedIndexes.reserve(components.size());
        }
        catch (const std::bad_alloc&)
        {
            CCTRACE("[CreateComponentsClouds] Not enough memory to sort components by size!");
            sortBysize = false;
        }

        if (sortBysize) //still ok?
        {
            unsigned compCount = static_cast<unsigned>(components.size());
            for (unsigned i = 0; i < compCount; ++i)
            {
                sortedIndexes.emplace_back(i, components[i]->size());
            }

            ParallelSort(sortedIndexes.begin(), sortedIndexes.end(), ComponentIndexAndSize::DescendingCompOperator);

            _sortedIndexes = &sortedIndexes;
        }
    }

    //we create "real" point clouds for all input components
    {
        ccPointCloud* pc = cloud->isA(CC_TYPES::POINT_CLOUD) ? static_cast<ccPointCloud*>(cloud) : nullptr;

        //for each component
        int nbComp =0;
        for (size_t i = 0; i < components.size(); ++i)
        {
            CCCoreLib::ReferenceCloud* compIndexes = _sortedIndexes ? components[_sortedIndexes->at(i).index] : components[i];

            //if it has enough points
            if (compIndexes->size() >= minPointsPerComponent)
            {
                //we create a new entity
                ccPointCloud* compCloud = (pc ? pc->partialClone(compIndexes) : ccPointCloud::From(compIndexes));
                if (compCloud)
                {
                    //shall we colorize it with random color?
                    if (randomColors)
                    {
                        ccColor::Rgb col = ccColor::Generator::Random();
                        compCloud->setColor(col);
                    }

                    //'shift on load' information
                    if (pc)
                    {
                        compCloud->copyGlobalShiftAndScale(*pc);
                    }
                    compCloud->setName(QString("CC#%1").arg(nbComp));

                    //we add new CC to group
                    resultClouds.push_back(compCloud);
                    nbComp++;
                }
                else
                {
                    CCTRACE("[CreateComponentsClouds] Failed to create component " << nbComp << "(not enough memory)");
                }
            }

            delete compIndexes;
            compIndexes = nullptr;
        }

        components.clear();

        if (nbComp == 0)
        {
            CCTRACE("No component was created! Check the minimum size...");
        }
        else
        {
            CCTRACE("[CreateComponentsClouds] " << nbComp << " component(s) were created from cloud " << cloud->getName().toStdString());
        }
    }
    return resultClouds;
}

bp::tuple ExtractConnectedComponents_py(std::vector<ccHObject*> entities,
                                        int octreeLevel=8,
                                        int minComponentSize=100,
                                        int maxNumberComponents=100,
                                        bool randomColors=false)
{
    CCTRACE("ExtractConnectedComponents_py");
    int realComponentCount = 0;
    int nbCloudDone = 0;

    std::vector<ccHObject*> resultComponents;
    bp::tuple res = bp::make_tuple(nbCloudDone, resultComponents);

    std::vector<ccGenericPointCloud*> clouds;
    {
        for ( ccHObject *entity : entities )
        {
            if (entity->isKindOf(CC_TYPES::POINT_CLOUD))
                clouds.push_back(ccHObjectCaster::ToGenericPointCloud(entity));
        }
    }

    size_t count = clouds.size();
    if (count == 0)
        return res;

    bool randColors = randomColors;

    for ( ccGenericPointCloud *cloud : clouds )
    {
        if (cloud && cloud->isA(CC_TYPES::POINT_CLOUD))
        {
            ccPointCloud* pc = static_cast<ccPointCloud*>(cloud);

            ccOctree::Shared theOctree = cloud->getOctree();
            if (!theOctree)
            {
                theOctree = cloud->computeOctree(nullptr);
                if (!theOctree)
                {
                    CCTRACE("Couldn't compute octree for cloud " <<cloud->getName().toStdString());
                    break;
                }
            }

            //we create/activate CCs label scalar field
            int sfIdx = pc->getScalarFieldIndexByName(CC_CONNECTED_COMPONENTS_DEFAULT_LABEL_NAME);
            if (sfIdx < 0)
            {
                sfIdx = pc->addScalarField(CC_CONNECTED_COMPONENTS_DEFAULT_LABEL_NAME);
            }
            if (sfIdx < 0)
            {
                CCTRACE("Couldn't allocate a new scalar field for computing CC labels! Try to free some memory ...");
                break;
            }
            pc->setCurrentScalarField(sfIdx);

            //we try to label all CCs
            CCCoreLib::ReferenceCloudContainer components;
            int componentCount = CCCoreLib::AutoSegmentationTools::labelConnectedComponents(cloud,
                                                                                        static_cast<unsigned char>(octreeLevel),
                                                                                        false,
                                                                                        nullptr,
                                                                                        theOctree.data());

            if (componentCount >= 0)
            {
                //if successful, we extract each CC (stored in "components")

                //safety test
                {
                    for (size_t i = 0; i < components.size(); ++i)
                    {
                        if (components[i]->size() >= minComponentSize)
                        {
                            ++realComponentCount;
                        }
                    }
                }

                if (realComponentCount > maxNumberComponents)
                {
                    //too many components
                    CCTRACE("Too many components: " << realComponentCount << " for a maximum of: " << maxNumberComponents);
                    CCTRACE("Extraction incomplete, modify some parameters and retry");
                    pc->deleteScalarField(sfIdx);
                    res = bp::make_tuple(nbCloudDone, resultComponents);
                    return res;
                }

                pc->getCurrentInScalarField()->computeMinAndMax();
                if (!CCCoreLib::AutoSegmentationTools::extractConnectedComponents(cloud, components))
                {
                    CCTRACE("[ExtractConnectedComponents] Something went wrong while extracting CCs from cloud " << cloud->getName().toStdString());
                }
            }
            else
            {
                CCTRACE("[ExtractConnectedComponents] Something went wrong while extracting CCs from cloud " << cloud->getName().toStdString());
            }

            //we delete the CCs label scalar field (we don't need it anymore)
            pc->deleteScalarField(sfIdx);
            sfIdx = -1;

            //we create "real" point clouds for all CCs
            if (!components.empty())
            {
                std::vector<ccPointCloud*> resultClouds = createComponentsClouds_(cloud, components, minComponentSize, randColors, true);
                for (ccPointCloud* cloud : resultClouds)
                    resultComponents.push_back(cloud);
            }
            nbCloudDone++;

        }
    }
    res = bp::make_tuple(nbCloudDone, resultComponents);
    return res;
}



BOOST_PYTHON_FUNCTION_OVERLOADS(importFilePy_overloads, importFilePy, 1, 5);
BOOST_PYTHON_FUNCTION_OVERLOADS(loadPointCloudPy_overloads, loadPointCloudPy, 1, 6);
BOOST_PYTHON_FUNCTION_OVERLOADS(loadMeshPy_overloads, loadMeshPy, 1, 6);
BOOST_PYTHON_FUNCTION_OVERLOADS(loadPolyline_overloads, loadPolyline, 1, 6);
BOOST_PYTHON_FUNCTION_OVERLOADS(GetPointCloudRadius_overloads, GetPointCloudRadius, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(ICP_py_overloads, ICP_py, 8, 13);
BOOST_PYTHON_FUNCTION_OVERLOADS(computeNormals_overloads, computeNormals, 1, 12);
BOOST_PYTHON_FUNCTION_OVERLOADS(RasterizeToCloud_overloads, RasterizeToCloud, 2, 19);
BOOST_PYTHON_FUNCTION_OVERLOADS(RasterizeToMesh_overloads, RasterizeToMesh, 2, 19);
BOOST_PYTHON_FUNCTION_OVERLOADS(RasterizeGeoTiffOnly_overloads, RasterizeGeoTiffOnly, 2, 19);
BOOST_PYTHON_FUNCTION_OVERLOADS(ExtractSlicesAndContours_py_overloads, ExtractSlicesAndContours_py, 2, 18);
BOOST_PYTHON_FUNCTION_OVERLOADS(ExtractConnectedComponents_py_overloads, ExtractConnectedComponents_py, 1, 5);


BOOST_PYTHON_MODULE(_cloudComPy)
{
    using namespace boost::python;

    bnp::initialize();
    initializeConverters();

    export_colors();
    export_ScalarField();
    export_ccGenericCloud();
    export_ccPolyline();
    export_ccOctree();
    export_ccPointCloud();
    export_ccMesh();
    export_ccPrimitives();
    export_distanceComputationTools();
    export_geometricalAnalysisTools();
    export_registrationTools();
    export_cloudSamplingTools();
    export_ccFacet();
    export_Neighbourhood();

    // TODO: function load entities ("file.bin")
    // TODO: more methods on distanceComputationTools
    // TODO: methods from ccEntityAction.h to transpose without dialogs
    // TODO: explore menus edit, tools, plugins
    // TODO: parameters on save mesh or clouds
    // TODO: 2D Polygon (cf. ccFacet.h)           <== issue Github
    // TODO: save histogram as .csv or .png       <== issue Github

    scope().attr("__doc__") = cloudComPy_doc;

    def("greet", greet);

    enum_<CC_SHIFT_MODE>("CC_SHIFT_MODE")
        .value("AUTO", AUTO)
        .value("XYZ", XYZ)
        .value("FIRST_GLOBAL_SHIFT", FIRST_GLOBAL_SHIFT)
        .value("NO_GLOBAL_SHIFT", NO_GLOBAL_SHIFT)
        ;

	enum_<CC_DIRECTION>("CC_DIRECTION")
		.value("X", CC_DIRECTION::X)
		.value("Y", CC_DIRECTION::Y)
		.value("Z", CC_DIRECTION::Z)
		;

    enum_<CC_FILE_ERROR>("CC_FILE_ERROR")
        .value("CC_FERR_NO_ERROR", CC_FERR_NO_ERROR)
        .value("CC_FERR_BAD_ARGUMENT", CC_FERR_BAD_ARGUMENT)
        .value("CC_FERR_UNKNOWN_FILE", CC_FERR_UNKNOWN_FILE)
        .value("CC_FERR_WRONG_FILE_TYPE", CC_FERR_WRONG_FILE_TYPE)
        .value("CC_FERR_WRITING", CC_FERR_WRITING)
        .value("CC_FERR_READING", CC_FERR_READING)
        .value("CC_FERR_NO_SAVE", CC_FERR_NO_SAVE)
        .value("CC_FERR_NO_LOAD", CC_FERR_NO_LOAD)
        .value("CC_FERR_BAD_ENTITY_TYPE", CC_FERR_BAD_ENTITY_TYPE)
        .value("CC_FERR_CANCELED_BY_USER", CC_FERR_CANCELED_BY_USER)
        .value("CC_FERR_NOT_ENOUGH_MEMORY", CC_FERR_NOT_ENOUGH_MEMORY)
        .value("CC_FERR_MALFORMED_FILE", CC_FERR_MALFORMED_FILE)
        .value("CC_FERR_CONSOLE_ERROR", CC_FERR_CONSOLE_ERROR)
        .value("CC_FERR_BROKEN_DEPENDENCY_ERROR", CC_FERR_BROKEN_DEPENDENCY_ERROR)
        .value("CC_FERR_FILE_WAS_WRITTEN_BY_UNKNOWN_PLUGIN", CC_FERR_FILE_WAS_WRITTEN_BY_UNKNOWN_PLUGIN)
        .value("CC_FERR_THIRD_PARTY_LIB_FAILURE", CC_FERR_THIRD_PARTY_LIB_FAILURE)
        .value("CC_FERR_THIRD_PARTY_LIB_EXCEPTION", CC_FERR_THIRD_PARTY_LIB_EXCEPTION)
        .value("CC_FERR_NOT_IMPLEMENTED", CC_FERR_NOT_IMPLEMENTED)
        ;

    enum_<CurvatureType>("CurvatureType")
        .value("GAUSSIAN_CURV", GAUSSIAN_CURV)
        .value("MEAN_CURV", MEAN_CURV)
        .value("NORMAL_CHANGE_RATE", NORMAL_CHANGE_RATE)
        ;

    enum_<CCCoreLib::LOCAL_MODEL_TYPES>("LOCAL_MODEL_TYPES")
        .value("NO_MODEL", CCCoreLib::NO_MODEL )
        .value("LS", CCCoreLib::LS )
        .value("TRI", CCCoreLib::TRI )
        .value("QUADRIC", CCCoreLib::QUADRIC )
        ;

    enum_<ccNormalVectors::Orientation>("Orientation")
        .value("PLUS_X", ccNormalVectors::PLUS_X )
        .value("MINUS_X", ccNormalVectors::MINUS_X )
        .value("PLUS_Y", ccNormalVectors::PLUS_Y )
        .value("MINUS_Y", ccNormalVectors::MINUS_Y )
        .value("PLUS_Z", ccNormalVectors::PLUS_Z )
        .value("MINUS_Z", ccNormalVectors::MINUS_Z )
        .value("PLUS_BARYCENTER", ccNormalVectors::PLUS_BARYCENTER )
        .value("MINUS_BARYCENTER", ccNormalVectors::MINUS_BARYCENTER )
        .value("PLUS_ORIGIN", ccNormalVectors::PLUS_ORIGIN )
        .value("MINUS_ORIGIN", ccNormalVectors::MINUS_ORIGIN )
        .value("PREVIOUS", ccNormalVectors::PREVIOUS )
        .value("PLUS_SENSOR_ORIGIN", ccNormalVectors::PLUS_SENSOR_ORIGIN )
        .value("MINUS_SENSOR_ORIGIN", ccNormalVectors::MINUS_SENSOR_ORIGIN )
        .value("UNDEFINED", ccNormalVectors::UNDEFINED )
        ;

	enum_<ccRasterGrid::ProjectionType>("ProjectionType")
		.value("PROJ_MINIMUM_VALUE", ccRasterGrid::PROJ_MINIMUM_VALUE)
		.value("PROJ_AVERAGE_VALUE", ccRasterGrid::PROJ_AVERAGE_VALUE)
		.value("PROJ_MAXIMUM_VALUE", ccRasterGrid::PROJ_MAXIMUM_VALUE)
		.value("INVALID_PROJECTION_TYPE", ccRasterGrid::INVALID_PROJECTION_TYPE)
		;

	enum_<ccRasterGrid::EmptyCellFillOption>("EmptyCellFillOption")
		.value("LEAVE_EMPTY", ccRasterGrid::LEAVE_EMPTY)
		.value("FILL_MINIMUM_HEIGHT", ccRasterGrid::FILL_MINIMUM_HEIGHT)
		.value("FILL_MAXIMUM_HEIGHT", ccRasterGrid::FILL_MAXIMUM_HEIGHT)
		.value("FILL_CUSTOM_HEIGHT", ccRasterGrid::FILL_CUSTOM_HEIGHT)
		.value("FILL_AVERAGE_HEIGHT", ccRasterGrid::FILL_AVERAGE_HEIGHT)
		.value("INTERPOLATE", ccRasterGrid::INTERPOLATE)
		;

    enum_<ccRasterGrid::ExportableFields>("ExportableFields")
        .value("PER_CELL_HEIGHT", ccRasterGrid::PER_CELL_HEIGHT)
        .value("PER_CELL_COUNT", ccRasterGrid::PER_CELL_COUNT)
        .value("PER_CELL_MIN_HEIGHT", ccRasterGrid::PER_CELL_MIN_HEIGHT)
        .value("PER_CELL_MAX_HEIGHT", ccRasterGrid::PER_CELL_MAX_HEIGHT)
        .value("PER_CELL_AVG_HEIGHT", ccRasterGrid::PER_CELL_AVG_HEIGHT)
        .value("PER_CELL_HEIGHT_STD_DEV", ccRasterGrid::PER_CELL_HEIGHT_STD_DEV)
        .value("PER_CELL_HEIGHT_RANGE", ccRasterGrid::PER_CELL_HEIGHT_RANGE)
        .value("PER_CELL_INVALID", ccRasterGrid::PER_CELL_INVALID)
        ;

    def("importFile", importFilePy,
        importFilePy_overloads((arg("filename"), arg("mode")=AUTO, arg("x")=0, arg("y")=0, arg("z")=0),
                               cloudComPy_importFile_doc));

    def("loadPointCloud", loadPointCloudPy,
        loadPointCloudPy_overloads((arg("filename"), arg("mode")=AUTO, arg("skip")=0, arg("x")=0, arg("y")=0, arg("z")=0),
                cloudComPy_loadPointCloud_doc)[return_value_policy<reference_existing_object>()]);

    def("loadMesh", loadMeshPy,
        loadMeshPy_overloads((arg("filename"), arg("mode")=AUTO, arg("skip")=0, arg("x")=0, arg("y")=0, arg("z")=0),
                cloudComPy_loadMesh_doc)[return_value_policy<reference_existing_object>()]);

    def("loadPolyline", loadPolyline,
        loadPolyline_overloads((arg("filename"), arg("mode")=AUTO, arg("skip")=0, arg("x")=0, arg("y")=0, arg("z")=0),
                               cloudComPy_loadPolyline_doc)
        [return_value_policy<reference_existing_object>()]);

    def("deleteEntity", deleteEntity, cloudComPy_deleteEntity_doc);

    def("SaveMesh", SaveMesh, cloudComPy_SaveMesh_doc);

    def("SavePointCloud", SavePointCloud, cloudComPy_SavePointCloud_doc);

    def("SaveEntities", SaveEntities, cloudComPy_SaveEntities_doc);

    def("initCC", &initCC_py, cloudComPy_initCC_doc);

    def("initCloudCompare", &initCloudCompare_py, cloudComPy_initCloudCompare_doc);

    def("isPluginDraco", &pyccPlugins::isPluginDraco, cloudComPy_isPluginDraco_doc);

    def("isPluginFbx", &pyccPlugins::isPluginFbx, cloudComPy_isPluginFbx_doc);

    def("isPluginHPR", &pyccPlugins::isPluginHPR, cloudComPy_isPluginHPR_doc);

    def("isPluginM3C2", &pyccPlugins::isPluginM3C2, cloudComPy_isPluginM3C2_doc);

    def("isPluginMeshBoolean", &pyccPlugins::isPluginMeshBoolean, cloudComPy_isPluginMeshBoolean_doc);

    def("isPluginPCL", &pyccPlugins::isPluginPCL, cloudComPy_isPluginPCL_doc);

    def("isPluginPCV", &pyccPlugins::isPluginPCV, cloudComPy_isPluginPCV_doc);

    def("isPluginRANSAC_SD", &pyccPlugins::isPluginRANSAC_SD, cloudComPy_isPluginRANSAC_SD_doc);

    def("computeCurvature", computeCurvature, cloudComPy_computeCurvature_doc);

    def("computeFeature", computeFeature, cloudComPy_computeFeature_doc);

    def("computeLocalDensity", computeLocalDensity, cloudComPy_computeLocalDensity_doc);

    def("computeApproxLocalDensity", computeApproxLocalDensity, cloudComPy_computeApproxLocalDensity_doc);

    def("computeRoughness", computeRoughness, cloudComPy_computeRoughness_doc);

    def("computeMomentOrder1", computeMomentOrder1, cloudComPy_computeMomentOrder1_doc);

#ifdef WRAP_PLUGIN_QM3C2
    def("computeM3C2", computeM3C2, return_value_policy<reference_existing_object>(), cloudComPy_computeM3C2_doc);
#endif

    def("filterBySFValue", filterBySFValue, return_value_policy<reference_existing_object>(), cloudComPy_filterBySFValue_doc);

    def("GetPointCloudRadius", GetPointCloudRadius,
        GetPointCloudRadius_overloads((arg("clouds"), arg("nodes")=12), cloudComPy_GetPointCloudRadius_doc));

    def("getScalarType", getScalarType, cloudComPy_getScalarType_doc);

    class_<ICPres>("ICPres", cloudComPy_ICPres_doc)
       .add_property("aligned", bp::make_getter(&ICPres::aligned, return_value_policy<return_by_value>()))
       .def_readwrite("transMat", &ICPres::transMat,
                       cloudComPy_ICPres_doc)
       .def_readwrite("finalScale", &ICPres::finalScale,
                      cloudComPy_ICPres_doc)
       .def_readwrite("finalRMS", &ICPres::finalRMS,
                      cloudComPy_ICPres_doc)
       .def_readwrite("finalPointCount", &ICPres::finalPointCount,
                      cloudComPy_ICPres_doc)
    ;

    def("ICP", ICP_py, ICP_py_overloads(
            (arg("data"), arg("model"), arg("minRMSDecrease"), arg("maxIterationCount"), arg("randomSamplingLimit"),
             arg("removeFarthestPoints"), arg("method"), arg("adjustScale"), arg("finalOverlapRatio")=1.0,
             arg("useDataSFAsWeights")=false, arg("useModelSFAsWeights")=false,
             arg("transformationFilters")=CCCoreLib::RegistrationTools::SKIP_NONE,
             arg("maxThreadCount")=0),
            cloudComPy_ICP_doc));

    def("computeNormals", computeNormals, computeNormals_overloads(
        (arg("selectedEntities"), arg("model")=CCCoreLib::LS,
         arg("useScanGridsForComputation")=true, arg("defaultRadius")=0.0, arg("minGridAngle_deg")=1.0,
         arg("orientNormals")=true, arg("useScanGridsForOrientation")=true,
         arg("useSensorsForOrientation")=true, arg("preferredOrientation")=ccNormalVectors::UNDEFINED,
         arg("orientNormalsMST")=true, arg("mstNeighbors")=6, arg("computePerVertexNormals")=true),
         cloudComPy_computeNormals_doc));

    class_<ReportInfoVol>("ReportInfoVol", cloudComPy_ReportInfoVol_doc)
		.def_readonly("volume", &ReportInfoVol::volume)
		.def_readonly("addedVolume", &ReportInfoVol::addedVolume)
		.def_readonly("removedVolume", &ReportInfoVol::removedVolume)
		.def_readonly("surface", &ReportInfoVol::surface)
		.def_readonly("matchingPercent", &ReportInfoVol::matchingPercent)
		.def_readonly("ceilNonMatchingPercent", &ReportInfoVol::ceilNonMatchingPercent)
		.def_readonly("groundNonMatchingPercent", &ReportInfoVol::groundNonMatchingPercent)
		.def_readonly("averageNeighborsPerCell", &ReportInfoVol::averageNeighborsPerCell)
		;

    def("ComputeVolume25D", ComputeVolume25D, cloudComPy_ComputeVolume25D_doc);

    def("invertNormals", invertNormals, cloudComPy_invertNormals_doc);

    def("ExtractConnectedComponents", ExtractConnectedComponents_py,
        ExtractConnectedComponents_py_overloads(
            (arg("clouds"),
             arg("octreeLevel")=8,
             arg("minComponentSize")=100,
             arg("maxNumberComponents")=100,
             arg("randomColors")=false),
            cloudComPy_ExtractConnectedComponents_doc));

    def("ExtractSlicesAndContours", ExtractSlicesAndContours_py,
        ExtractSlicesAndContours_py_overloads(
            (arg("entities"),
             arg("bbox"),
             arg("bboxTrans")=ccGLMatrix(),
             arg("singleSliceMode")=true,
             arg("processRepeatX")=false,
             arg("processRepeatY")=false,
             arg("processRepeatZ")=true,
             arg("extractEnvelopes")=false,
             arg("maxEdgeLength")=0,
             arg("envelopeType")=0,
             arg("extractLevelSet")=false,
             arg("levelSetGridStep")=0,
             arg("levelSetMinVertCount")=0,
             arg("gap")=0,
             arg("multiPass")=false,
             arg("splitEnvelopes")=false,
             arg("projectOnBestFitPlane")=false,
             arg("generateRandomColors")=false),
            cloudComPy_ExtractSlicesAndContours_doc));

    def("RasterizeToCloud", RasterizeToCloud,
		RasterizeToCloud_overloads(
    		(arg("cloud"),
    		 arg("gridStep"),
    		 arg("vertDir") = CC_DIRECTION::Z,
    		 arg("outputRasterZ")=false,
    		 arg("outputRasterSFs")=false,
    		 arg("outputRasterRGB")=false,
    		 arg("pathToImages")=".",
    		 arg("resample")=false,
    		 arg("projectionType")=ccRasterGrid::PROJ_AVERAGE_VALUE,
    		 arg("sfProjectionType")=ccRasterGrid::PROJ_AVERAGE_VALUE,
    		 arg("emptyCellFillStrategy")=ccRasterGrid::LEAVE_EMPTY,
    		 arg("customHeight")=std::numeric_limits<double>::quiet_NaN(),
    		 arg("gridBBox")=ccBBox(),
    		 arg("export_perCellCount")=false,
    		 arg("export_perCellMinHeight")=false,
    		 arg("export_perCellMaxHeight")=false,
    		 arg("export_perCellAvgHeight")=false,
    		 arg("export_perCellHeightStdDev")=false,
    		 arg("export_perCellHeightRange")=false),
    		cloudComPy_RasterizeToCloud_doc)[return_value_policy<reference_existing_object>()]);

    def("RasterizeToMesh", RasterizeToMesh,
    		RasterizeToMesh_overloads(
            (arg("cloud"),
             arg("gridStep"),
             arg("vertDir") = CC_DIRECTION::Z,
             arg("outputRasterZ")=false,
             arg("outputRasterSFs")=false,
             arg("outputRasterRGB")=false,
             arg("pathToImages")=".",
             arg("resample")=false,
             arg("projectionType")=ccRasterGrid::PROJ_AVERAGE_VALUE,
             arg("sfProjectionType")=ccRasterGrid::PROJ_AVERAGE_VALUE,
             arg("emptyCellFillStrategy")=ccRasterGrid::LEAVE_EMPTY,
             arg("customHeight")=std::numeric_limits<double>::quiet_NaN(),
             arg("gridBBox")=ccBBox(),
             arg("export_perCellCount")=false,
             arg("export_perCellMinHeight")=false,
             arg("export_perCellMaxHeight")=false,
             arg("export_perCellAvgHeight")=false,
             arg("export_perCellHeightStdDev")=false,
             arg("export_perCellHeightRange")=false),
            cloudComPy_RasterizeToMesh_doc)[return_value_policy<reference_existing_object>()]);

    def("RasterizeGeoTiffOnly", RasterizeGeoTiffOnly,
    		RasterizeGeoTiffOnly_overloads(
            (arg("cloud"),
             arg("gridStep"),
             arg("vertDir") = CC_DIRECTION::Z,
             arg("outputRasterZ")=false,
             arg("outputRasterSFs")=false,
             arg("outputRasterRGB")=false,
             arg("pathToImages")=".",
             arg("resample")=false,
             arg("projectionType")=ccRasterGrid::PROJ_AVERAGE_VALUE,
             arg("sfProjectionType")=ccRasterGrid::PROJ_AVERAGE_VALUE,
             arg("emptyCellFillStrategy")=ccRasterGrid::LEAVE_EMPTY,
             arg("customHeight")=std::numeric_limits<double>::quiet_NaN(),
             arg("gridBBox")=ccBBox(),
             arg("export_perCellCount")=false,
             arg("export_perCellMinHeight")=false,
             arg("export_perCellMaxHeight")=false,
             arg("export_perCellAvgHeight")=false,
             arg("export_perCellHeightStdDev")=false,
             arg("export_perCellHeightRange")=false),
            cloudComPy_RasterizeGeoTiffOnly_doc)[return_value_policy<reference_existing_object>()]);

}
