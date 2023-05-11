//CODE WITH TRANSLATIONS + REFLEXION deID 825
#include "boost/program_options.hpp"
#include "MCHMappingInterface/CathodeSegmentation.h"
#include "MCHMappingInterface/Segmentation.h"
#include "MCHMappingSegContour/CathodeSegmentationContours.h"
#include "MCHMappingSegContour/CathodeSegmentationSVGWriter.h"
#include "MCHGeometryTransformer/Transformations.h"
#include "TGeoManager.h"
#include "MCHContour/SVGWriter.h"
#include "MCHConstants/DetectionElements.h"
#include <fstream>
#include <iostream>
#include "TFile.h"
#include "TDirectory.h"
#include "TList.h"
#include "TH1F.h"


using namespace o2::mch::mapping;

namespace po = boost::program_options;

std::pair<double, double> parsePoint(std::string ps)
{
  int ix = ps.find_first_of(' ');

  auto first = ps.substr(0, ix);
  auto second = ps.substr(ix + 1, ps.size() - ix - 1);
  return std::make_pair(std::stod(first), std::stod(second));
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Function with 156 translation offsets to not overlap the deId of each chamber:
std::pair<double, double> getTranslationOffset(int deId) {
    std::map<int, std::pair<double, double>> translationOffsets = {
        {100, {5.0, 0.0}}, {101, {-5.0, 0.0}}, {102, {-5.0, 10.0}}, {103, {5.0, 10.0}},
        {200, {5.0, 0.0}}, {201, {-5.0, 0.0}}, {202, {-5.0, 10.0}}, {203, {5.0, 10.0}},
        {300, {5.0, 0.0}}, {301, {-5.0, 0.0}}, {302, {-5.0, 10.0}}, {303, {5.0, 10.0}},
        {400, {5.0, 0.0}}, {401, {-5.0, 0.0}}, {402, {-5.0, 10.0}}, {403, {5.0, 10.0}},

        {500, {5.0, 5.0}}, {501, {5.0, -10.0}}, {502, {5.0, -20.0}}, {503, {5.0, -30.0}},
        {504, {5.0, -40.0}}, {505, {0, -40.0}}, {506, {0, -30.0}}, {507, {0, -20.0}}, {508, {0, -10.0}},
        {509, {0.0, 5.0}}, {510, {0.0, 20.0}}, {511, {0.0, 30.0}}, {512, {0.0, 40.0}},
        {513, {0.0, 50.0}}, {514, {5.0, 50.0}}, {515, {5.0, 40.0}}, {516, {5.0, 30.0}}, {517, {5.0, 20.0}},
        {600, {5.0, 5.0}}, {601, {5.0, -10.0}}, {602, {5.0, -20.0}}, {603, {5.0, -30.0}},
        {604, {5.0, -40.0}}, {605, {0, -40.0}}, {606, {0, -30.0}}, {607, {0, -20.0}}, {608, {0, -10.0}},
        {609, {0.0, 5.0}}, {610, {0.0, 20.0}}, {611, {0.0, 30.0}}, {612, {0.0, 40.0}},
        {613, {0.0, 50.0}}, {614, {5.0, 50.0}}, {615, {5.0, 40.0}}, {616, {5.0, 30.0}}, {617, {5.0, 20.0}},


        {700, {3.0, 5.0}}, {701, {3.0, -10.0}}, {702, {3.0, -20.0}}, {703, {3.0, -30.0}}, {704, {3.0, -50.0}}, {705, {3, -60.0}}, {706, {3, -80.0}}, {707, {0, -80.0}}, {708, {0, -60.0}}, {709, {0.0, -50}}, {710, {0.0, -30.0}}, {711, {0.0, -20.0}}, {712, {0.0, -10.0}}, {713, {0.0, 5.0}}, {714, {0.0, 20.0}}, {715, {0.0, 30.0}}, {716, {0.0, 42.0}}, {717, {0.0, 64.0}}, {718, {0.0, 79.0}}, {719, {0.0, 101.0}}, {720, {3.0, 101.0}}, {721, {3.0, 79}}, {722, {3.0, 64.0}}, {723, {3.0, 42.0}}, {724, {3.0, 30}}, {725, {3.0, 20}},
        {800, {3.0, 5.0}}, {801, {3.0, -10.0}}, {802, {3.0, -20.0}}, {803, {3.0, -30.0}}, {804, {3.0, -50.0}}, {805, {3, -60.0}}, {806, {3, -80.0}}, {807, {0, -80.0}}, {808, {0, -60.0}}, {809, {0.0, -50}}, {810, {0.0, -30.0}}, {811, {0.0, -20.0}}, {812, {0.0, -10.0}}, {813, {0.0, 5.0}}, {814, {0.0, 20.0}}, {815, {0.0, 30.0}}, {816, {0.0, 42.0}}, {817, {0.0, 64.0}}, {818, {0.0, 79.0}}, {819, {0.0, 101.0}}, {820, {3.0, 101.0}}, {821, {3.0, 79}}, {822, {3.0, 64.0}}, {823, {3.0, 42.0}}, {824, {3.0, 30}}, {825, {3.0, 97}},  // deId 825 has an extra reflexion, this is why the value is adjusted like this..
        {900, {0.0, 5.0}}, {901, {0.0, -10.0}}, {902, {0.0, -20.0}}, {903, {0.0, -30.0}}, {904, {0.0, -50.0}}, {905, {0, -60.0}}, {906, {0, -80.0}}, {907, {0, -80.0}}, {908, {0, -60.0}}, {909, {0.0, -50}}, {910, {0.0, -30.0}}, {911, {0.0, -20.0}}, {912, {0.0, -10.0}}, {913, {0.0, 5.0}}, {914, {0.0, 20.0}}, {915, {0.0, 30.0}}, {916, {0.0, 50.0}}, {917, {0.0, 70.0}}, {918, {0.0, 90.0}}, {919, {0.0, 110.0}}, {920, {0.0, 110.0}}, {921, {0.0, 90}}, {922, {0.0, 70}}, {923, {0.0, 50}}, {924, {0.0, 30}}, {925, {0.0, 20}},
        {1000, {0.0, 5.0}}, {1001, {0.0, -10.0}}, {1002, {0.0, -20.0}}, {1003, {0.0, -30.0}}, {1004, {0.0, -50.0}}, {1005, {0, -60.0}}, {1006, {0, -80.0}}, {1007, {0, -80.0}}, {1008, {0, -60.0}}, {1009, {0.0, -50}}, {1010, {0.0, -30.0}}, {1011, {0.0, -20.0}}, {1012, {0.0, -10.0}}, {1013, {0.0, 5.0}}, {1014, {0.0, 20.0}}, {1015, {0.0, 30.0}}, {1016, {0.0, 50.0}}, {1017, {0.0, 70.0}}, {1018, {0.0, 90.0}}, {1019, {0.0, 110.0}}, {1020, {0.0, 110.0}}, {1021, {0.0, 90}}, {1022, {0.0, 70}}, {1023, {0.0, 50}}, {1024, {0.0, 30}}, {1025, {0.0, 20}}
       
    };
    
        return translationOffsets[deId];
    
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Function to transform local contours to global coordinates
std::vector<o2::mch::contour::Contour<double>> transformLocalToGlobal(int deId, bool bending, const o2::mch::geo::TransformationCreator& transformation) {
    CathodeSegmentation cSeg{deId, bending}; // Assuming there is a function to get cathode segmentation from deID
    std::vector<o2::mch::contour::Contour<double>> dualSampaContoursIn = getDualSampaContours(cSeg);
    std::vector<o2::mch::contour::Contour<double>> dualSampaContoursOut; 
    auto t = transformation(deId);

    for(int i = 0; i < dualSampaContoursIn.size(); i++) {
        auto dsContour = dualSampaContoursIn[i];
        o2::mch::contour::Contour<double> dsContour2;

        for (auto p = 0; p < dsContour.size(); p++) {
            auto polyg = dsContour[p];
            std::vector<o2::mch::contour::Vertex<double>> vertices;

            for (auto v = 0; v < polyg.size(); v++) {
                auto vertex = polyg[v];
                o2::math_utils::Point3D<float> lpos(vertex.x, vertex.y, 0.0);
                o2::math_utils::Point3D<float> gpos;

                //Rotations + Translations: local --> global  
                t.LocalToMaster(lpos, gpos);
                std::cout << "vertex" << vertex << std::endl;
                o2::mch::contour::Vertex<double> v2;

     // Apply extra reflection for deID 825 + Extra Translations for all deId to not overlap
     
                auto offset = getTranslationOffset(deId);
                   
                 if (deId == 825) {
                    v2.x = gpos.X() + offset.first;
                    v2.y = gpos.Y() + offset.second;   
                } else {
                    v2.y = -gpos.Y();    // reflection anti-clockwise (minus value)
                    v2.y += offset.second;
                    v2.x = gpos.X() + offset.first;
                }   
             
                vertices.push_back(v2);
                std::cout << "vertex2" << vertices[v] << std::endl;  
            }

            o2::mch::contour::Polygon<double> polyg2(vertices.begin(), vertices.end()); 
            std::cout << "polyg2" << polyg2 << std::endl;
            dsContour2.addPolygon(polyg2);
            std::cout << "dscontour2" << dsContour2[p] << std::endl;
        }

        dualSampaContoursOut.push_back(dsContour2);
    }


    return dualSampaContoursOut;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// GET ALL deID of one chamber:
std::vector<int> getAllDeIds(int nChamber) {
    std::vector<int> deIds;
    int i_1 = *std::move(o2::mch::constants::deId2DeIndex(nChamber*100));    //move solve the problem of optional integer...
    int i_2 = *std::move(o2::mch::constants::deId2DeIndex((nChamber+1)*100));
    int Chamberlength = i_2 -i_1;

    if(nChamber<10){
        for (auto i = 0; i < Chamberlength; i++) {
            deIds.push_back(o2::mch::constants::deIdsForAllMCH[i+i_1]);
        }
    }
    else{
        for (int i = 0; i < 26; i++) {
            deIds.push_back(o2::mch::constants::deIdsForAllMCH[i+i_1]);
        }
    }
    return deIds;
}

////////////////////////////////////////

// GET ALL DualSampa of one deId:
std::vector<int> getDualSampas(int deId) {
    std::vector<int> dualSampas;
    const o2::mch::mapping::Segmentation& seg = o2::mch::mapping::segmentation(deId);
    seg.forEachDualSampa([&dualSampas](int ds) { dualSampas.push_back(ds); });
    return dualSampas;  //  Dual Sampa ID
}


// Get All Dual Sampas of n Chambers:
std::vector<int> getAllDualSampas(int numChambers) {
    std::vector<int> allDualSampas;
    int count = 0; // Initialize count to 0

    for (int nChamber = 1; nChamber <= numChambers; nChamber++) {
        std::vector<int> deIds = getAllDeIds(nChamber);
        std::vector<int> dualSampas;
        for (auto deId : deIds) {
            std::vector<int> dsIds = getDualSampas(deId);
            for (auto dsId : dsIds) {
                dsId += count; 
                dualSampas.push_back(dsId);
            }
        }
        count = dualSampas.back() + 1; // Increment the count with the last value + 1:
        allDualSampas.insert(allDualSampas.end(), dualSampas.begin(), dualSampas.end());
    }


    std::cout <<"AAll Dualsampas of Chambers=";
    for (int i = 0; i < allDualSampas.size(); i++) {
        std::cout << allDualSampas[i] << ", ";
    }
    std::cout << std::endl;

    return allDualSampas;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void svgChamber(o2::mch::contour::SVGWriter& w, int nChamber, bool bending) {

/////////////////////// Lecture file from Quality Control ClustersMCH.root

    TFile* file = TFile::Open("/Users/valencia/test/ClustersMCH.root");
    TH1F* ClustersperDualSampa = (TH1F*)file->Get("ClustersPerDualSampa");

    std::vector<int> nClusters;
    std::vector<int> dsindex;

    int nclustermax = ClustersperDualSampa->GetNbinsX();
    std::cout << "clustersssss " << nclustermax << std::endl;

    for (int i = 1; i <= nclustermax; i++) {
        int clusters = ClustersperDualSampa->GetBinContent(i);
        if (clusters > 0) { // Only consider non-zero bins
            nClusters.push_back(clusters);
            dsindex.push_back(i-1); // Subtract 1 to get the dsindex
        }
    }

    for (int i = 0; i < nClusters.size(); i++) {
    std::cout << "dindex: " << dsindex[i] << ", nClusters: " << nClusters[i] << std::endl;
    }

/////////////////////////////////  Colors  ////////////////////////////////////////////////

    std::vector<std::string> colors = {"white","red"};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //Getting transformation from .json
    std::ifstream in("o2sim_geometry-aligned.json");
    auto transformation = o2::mch::geo::transformationFromJSON(in);

  
    //Getting all deIds for all Chambers, info found in DetectionElements.h
    auto deIds = getAllDeIds(nChamber);

    double epsilon=1.e-6;
    // Contours of all deId transformated  + SVGWRITER of all dsContourOut
    for (auto deId : deIds) {
        auto dualSampaContoursOut = transformLocalToGlobal(deId, bending, transformation);

        w.svgGroupStart("dualsampas");

        for (auto i=0;  i < dualSampaContoursOut.size();i++) {
                std::string  str = ".dualsampas { fill:";
                str += colors[0]; // Assign the first color in the vector
                str += "; stroke-width: 0.25px; stroke: #333333;}";
                w.addStyle(str);
                //int colorId = int( nClusters[i] / (nclustermax - epsilon) * colors.size());
                int colorId = i % colors.size();
                w.contour(dualSampaContoursOut[i], colors[colorId]);

            
        }


        w.svgGroupEnd();
    }
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




int main(int argc, char* argv[])
{

  std::string prefix;
  std::vector<int> detElemIds; 
  std::vector<int> dualSampas; 

  using Point = std::pair<double, double>;
  std::vector<std::string> pointStrings;
  std::vector<Point> points;
  po::variables_map vm;
  po::options_description generic("Generic options");

  generic.add_options()("help", "produce help message")("hidepads", "hide pad outlines")(
    "hidedualsampas", "hide dualsampa outlines")("hidedes", "hide detection element outline")(
    "hidepadchannels", "hide pad channel numbering")("de", po::value<std::vector<int>>(&detElemIds),
                                                     "which detection element to consider")(
    "prefix", po::value<std::string>(&prefix)->default_value("seg"), "prefix used for outfile filename(s)")(
    "point", po::value<std::vector<std::string>>(&pointStrings), "points to show")("all", "use all detection elements");

  po::options_description cmdline;
  cmdline.add(generic);

  po::store(po::command_line_parser(argc, argv).options(cmdline).run(), vm);
  po::notify(vm);

    std::cout << "deId:" << detElemIds[0] << "\n";

  if (vm.count("help")) {
    std::cout << generic << "\n";
    return 2;
  }

  if (vm.count("de") && vm.count("all")) {
    std::cout << "--all and --de options are mutually exclusive. --all will be used\n";
    detElemIds.clear();
  }

  if (vm.count("all")) {
    o2::mch::mapping::forOneDetectionElementOfEachSegmentationType(
      [&detElemIds](int detElemId) { detElemIds.push_back(detElemId); });
  }


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 // Get All dualSampas for 10 chambers with Global Index:

getAllDualSampas(10);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Define the bounding boxes for the 10 images:
std::vector<o2::mch::contour::BBox<double>> bboxes = {
    {-100, -100, 100, 100},
    {-100, -100, 100, 100},
    {-135, -135, 135, 135},
    {-135, -135, 135, 135},
    {-240, -240, 240, 240},
    {-240, -240, 240, 240},
    {-350, -350, 350, 350},
    {-375, -370, 375, 375},
    {-375, -370, 375, 375},
    {-375, -370, 375, 375}
};

// Create the SVGWriterNew objects for the 10 images with their respective bounding boxes
for (auto isBendingPlane : {true, false}) {
    for (int i = 0; i < 10; i++) {
        std::ofstream outv("CHAMBERS_" + std::to_string(i+1) + "-" +
                        (isBendingPlane ? "B" : "NB") + "translated.html");

        // Create the SVGWriterNew object:
        o2::mch::contour::SVGWriter wSeg(bboxes[i]);

        svgChamber(wSeg, i+1, isBendingPlane);
        
        // Write the HTML for the chamber to the output file, wrapped in a <div> tag
        outv << "<div style='display:flex;justify-content:center'>" << std::endl;
        wSeg.writeHTML(outv);
        outv << "<div style='margin-left:20px;'></div>" << std::endl;

        wSeg.writeHTML(outv);
        outv << "</div>" << std::endl;
   
    } 
}    



  return 0;
}
