#include "ant/server/default_path_handlers.h"
#include "ant/server/pprof_path_handlers.h"
#include "ant/server/web_server.h"

#include <sys/stat.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <glog/logging.h>

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <gperftools/malloc_extension.h>

#include "ant/base/map-util.h"
#include "ant/base/strings/human_readable.h"
#include "ant/base/strings/split.h"
#include "ant/base/strings/substitute.h"
//#include "ant/util/flag_tags.h"
#include "ant/util/histogram.pb.h"
//#include "ant/util/logging.h"
#include "ant/util/mem_tracker.h"
#include "ant/util/metrics.h"
#include "ant/util/jsonwriter.h"

using boost::replace_all;
using google::CommandlineFlagsIntoString;
using std::ifstream;
using std::string;
using std::endl;
using strings::Substitute;

DEFINE_int64(web_log_bytes, 1024 * 1024,
    "The maximum number of bytes to display on the debug webserver's log page");
#if 0
TAG_FLAG(web_log_bytes, advanced);
TAG_FLAG(web_log_bytes, runtime);
#endif

//TODO(wqx):
DEFINE_string(log_filename, "",
              "Prefix of log filename - "
              "full path is <log_dir>/<log_filename>.[INFO|WARN|ERROR|FATAL]");

static void GetFullLogFilename(google::LogSeverity severity, string* filename) {
  std::ostringstream ss;
  ss << FLAGS_log_dir << "/" << FLAGS_log_filename << "."
     << google::GetLogSeverityName(severity);
  *filename = ss.str();
}

namespace ant {

using std::shared_ptr;

namespace {
// Html/Text formatting tags
struct Tags {
  string pre_tag, end_pre_tag, line_break, header, end_header;

  // If as_text is true, set the html tags to a corresponding raw text representation.
  explicit Tags(bool as_text) {
    if (as_text) {
      pre_tag = "";
      end_pre_tag = "\n";
      line_break = "\n";
      header = "";
      end_header = "";
    } else {
      pre_tag = "<pre>";
      end_pre_tag = "</pre>";
      line_break = "<br/>";
      header = "<h2>";
      end_header = "</h2>";
    }
  }
};
} // anonymous namespace

// Writes the last FLAGS_web_log_bytes of the INFO logfile to a webpage
// Note to get best performance, set GLOG_logbuflevel=-1 to prevent log buffering
static void LogsHandler(const WebServer::WebRequest& req, std::ostringstream* output) {
  bool as_text = (req.parsed_args.find("raw") != req.parsed_args.end());
  Tags tags(as_text);
  string logfile;
  GetFullLogFilename(google::INFO, &logfile);
  (*output) << tags.header <<"INFO logs" << tags.end_header << endl;
  (*output) << "Log path is: " << logfile << endl;

  struct stat file_stat;
  if (stat(logfile.c_str(), &file_stat) == 0) {
    size_t size = file_stat.st_size;
    size_t seekpos = size < FLAGS_web_log_bytes ? 0L : size - FLAGS_web_log_bytes;
    ifstream log(logfile.c_str(), std::ios::in);
    // Note if the file rolls between stat and seek, this could fail
    // (and we could wind up reading the whole file). But because the
    // file is likely to be small, this is unlikely to be an issue in
    // practice.
    log.seekg(seekpos);
    (*output) << tags.line_break <<"Showing last " << FLAGS_web_log_bytes
              << " bytes of log" << endl;
    (*output) << tags.line_break << tags.pre_tag << log.rdbuf() << tags.end_pre_tag;

  } else {
    (*output) << tags.line_break << "Couldn't open INFO log file: " << logfile;
  }
}

// Registered to handle "/flags", and prints out all command-line flags and their values
static void FlagsHandler(const WebServer::WebRequest& req, std::ostringstream* output) {
  bool as_text = (req.parsed_args.find("raw") != req.parsed_args.end());
  Tags tags(as_text);
  (*output) << tags.header << "Command-line Flags" << tags.end_header;
  (*output) << tags.pre_tag << CommandlineFlagsIntoString() << tags.end_pre_tag;
}

// Registered to handle "/memz", and prints out memory allocation statistics.
static void MemUsageHandler(const WebServer::WebRequest& req, std::ostringstream* output) {
  bool as_text = (req.parsed_args.find("raw") != req.parsed_args.end());
  Tags tags(as_text);

  (*output) << tags.pre_tag;
#ifndef TCMALLOC_ENABLED
  (*output) << "Memory tracking is not available unless tcmalloc is enabled.";
#else
  char buf[2048];
  MallocExtension::instance()->GetStats(buf, 2048);
  // Replace new lines with <br> for html
  string tmp(buf);
  replace_all(tmp, "\n", tags.line_break);
  (*output) << tmp << tags.end_pre_tag;
#endif
}

// Registered to handle "/mem-trackers", and prints out to handle memory tracker information.
static void MemTrackersHandler(const WebServer::WebRequest& req, std::ostringstream* output) {
  *output << "<h1>Memory usage by subsystem</h1>\n";
  *output << "<table class='table table-striped'>\n";
  *output << "  <tr><th>Id</th><th>Parent</th><th>Limit</th><th>Current Consumption</th>"
      "<th>Peak consumption</th></tr>\n";

  vector<shared_ptr<MemTracker> > trackers;
  MemTracker::ListTrackers(&trackers);
  for (const shared_ptr<MemTracker>& tracker : trackers) {
    string parent = tracker->parent() == nullptr ? "none" : tracker->parent()->id();
    string limit_str = tracker->limit() == -1 ? "none" :
                       HumanReadableNumBytes::ToString(tracker->limit());
    string current_consumption_str = HumanReadableNumBytes::ToString(tracker->consumption());
    string peak_consumption_str = HumanReadableNumBytes::ToString(tracker->peak_consumption());
    (*output) << Substitute("  <tr><td>$0</td><td>$1</td><td>$2</td>" // id, parent, limit
                            "<td>$3</td><td>$4</td></tr>\n", // current, peak
                            tracker->id(), parent, limit_str, current_consumption_str,
                            peak_consumption_str);
  }
  *output << "</table>\n";
}

void AddDefaultPathHandlers(WebServer* webserver) {
  webserver->RegisterPathHandler("/logs", "Logs", LogsHandler);
  webserver->RegisterPathHandler("/varz", "Flags", FlagsHandler);
  webserver->RegisterPathHandler("/memz", "Memory (total)", MemUsageHandler);
  webserver->RegisterPathHandler("/mem-trackers", "Memory (detail)", MemTrackersHandler);

  AddPprofPathHandlers(webserver);
}


static void WriteMetricsAsJson(const MetricRegistry* const metrics,
                               const WebServer::WebRequest& req, std::ostringstream* output) {
  const string* requested_metrics_param = FindOrNull(req.parsed_args, "metrics");
  vector<string> requested_metrics;
  MetricJsonOptions opts;

  {
    string arg = FindWithDefault(req.parsed_args, "include_raw_histograms", "false");
    opts.include_raw_histograms = ParseLeadingBoolValue(arg.c_str(), false);
  }
  {
    string arg = FindWithDefault(req.parsed_args, "include_schema", "false");
    opts.include_schema_info = ParseLeadingBoolValue(arg.c_str(), false);
  }
  JsonWriter::Mode json_mode;
  {
    string arg = FindWithDefault(req.parsed_args, "compact", "false");
    json_mode = ParseLeadingBoolValue(arg.c_str(), false) ?
      JsonWriter::COMPACT : JsonWriter::PRETTY;
  }

  JsonWriter writer(output, json_mode);

  if (requested_metrics_param != nullptr) {
    SplitStringUsing(*requested_metrics_param, ",", &requested_metrics);
  } else {
    // Default to including all metrics.
    requested_metrics.push_back("*");
  }

  WARN_NOT_OK(metrics->WriteAsJson(&writer, requested_metrics, opts),
              "Couldn't write JSON metrics over HTTP");
}

void RegisterMetricsJsonHandler(WebServer* webserver, const MetricRegistry* const metrics) {
  WebServer::PathHandlerCallback callback = boost::bind(WriteMetricsAsJson, metrics, _1, _2);
  bool not_styled = false;
  bool not_on_nav_bar = false;
  bool is_on_nav_bar = true;
  webserver->RegisterPathHandler("/metrics", "Metrics", callback, not_styled, is_on_nav_bar);

  // The old name -- this is preserved for compatibility with older releases of
  // monitoring software which expects the old name.
  webserver->RegisterPathHandler("/jsonmetricz", "Metrics", callback, not_styled, not_on_nav_bar);
}

} // namespace ant
