//DO HISTOS OF AMBIGUOS TRACKS...

#include "Framework/runDataProcessing.h"
#include "Framework/AnalysisTask.h"
#include "Common/DataModel/TrackSelectionTables.h"
#include "Framework/ASoAHelpers.h"

using namespace o2;
using namespace o2::framework;
using namespace o2::framework::expressions;

namespace o2::aod
{
// helper for finding out which tracks are ambiguous
namespace helperSpace
{
// Global bool
DECLARE_SOA_COLUMN(IsAmbiguous, isAmbiguous, bool); //! is this ambiguous? 
}
DECLARE_SOA_TABLE(HelperTable, "AOD", "HELPERTABLE",
                  helperSpace::IsAmbiguous);
}

struct myExampleProducer {
  Produces<aod::HelperTable> myHelperTable;
  void init(InitContext const&) {}
  void process(o2::aod::Tracks const& tracks, o2::aod::AmbiguousTracks const& ambiTracks)
  {
    std::vector<bool> ambiguousOrNot;
    ambiguousOrNot.reserve(tracks.size());
    for (int ii = 0; ii < tracks.size(); ii++)
      ambiguousOrNot[ii] = false;
    for (auto const& ambiTrack : ambiTracks) {
      ambiguousOrNot[ambiTrack.trackId()] = true;
    }
    for (int ii = 0; ii < tracks.size(); ii++) {
      myHelperTable(ambiguousOrNot[ii]);
    }
  }
};

//This is an example of a convenient declaration of "using"
using myCompleteTracks = soa::Join<aod::Tracks, aod::TracksExtra, aod::TracksDCA, aod::HelperTable>;

struct myExampleTask {
  // Histogram registry: an object to hold your histograms
  HistogramRegistry histos{"histos", {}, OutputObjHandlingPolicy::AnalysisObject};

  Configurable<int> nBinsPt{"nBinsPt", 100, "N bins in pT histo"};

  Filter trackDCA = nabs(aod::track::dcaXY) < 0.2f;
  Filter trackIsAmbi = aod::helperSpace::isAmbiguous == true;
  using myFilteredTracks = soa::Filtered<myCompleteTracks>;

  void init(InitContext const&)
  {
    // define axes you want to use
    const AxisSpec axisCounter{1, 0, +1, ""};
    const AxisSpec axisEta{30, -1.5, +1.5, "#eta"};
    const AxisSpec axisPt{nBinsPt, 0, 10, "p_{T}"};
    // create histograms
    histos.add("eventCounter", "eventCounter", kTH1F, {axisCounter});
    histos.add("etaHistogramAmbi", "etaHistogramAmbi", kTH1F, {axisEta});
    histos.add("ptHistogramAmbi", "ptHistogramAmbi", kTH1F, {axisPt});
  }

  void process(myFilteredTracks const& tracks, o2::aod::AmbiguousTracks const& ambiTracks)
  {
    histos.fill(HIST("eventCounter"), 0.5);
    for (const auto& track : tracks) {
      if( track.tpcNClsCrossedRows() < 70 ) continue; //badly tracked
      histos.fill(HIST("etaHistogramAmbi"), track.eta());
      histos.fill(HIST("ptHistogramAmbi"), track.pt());
    }
  }
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  return WorkflowSpec{
    adaptAnalysisTask<myExampleProducer>(cfgc),
    adaptAnalysisTask<myExampleTask>(cfgc)
    };
}
