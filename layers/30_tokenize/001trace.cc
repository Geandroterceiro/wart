using std::pair;

struct trace_stream {
  vector<pair<string, string> > past_lines;   // [(layer label, line)]
  // accumulator for current line
  ostringstream* curr_stream;
  string curr_layer;
  trace_stream() :curr_stream(NULL) {}
  ~trace_stream() { if (curr_stream) delete curr_stream; }

  ostringstream& stream(string layer) {
    reset();
    curr_stream = new ostringstream;
    curr_layer = layer;
    return *curr_stream;
  }

  string contents(string layer) {
    reset();
    ostringstream output;
    for (vector<pair<string, string> >::iterator p = past_lines.begin(); p != past_lines.end(); ++p)
      if (layer.empty() || p->first == layer)
        output << p->second;
    return output.str();
  }

  void reset() {
    if (!curr_stream) return;
    past_lines.push_back(pair<string, string>(curr_layer, curr_stream->str()));
    delete curr_stream;
    curr_stream = NULL;
  }
};

trace_stream* global_trace_stream = NULL;

#define trace(layer) !global_trace_stream ? cerr : global_trace_stream->stream(layer)

// global_trace_stream is a resource, lease_tracer uses RAII to manage it.
struct lease_tracer {
  lease_tracer() { global_trace_stream = new trace_stream; }
  ~lease_tracer() { delete global_trace_stream, global_trace_stream = NULL; }
};
#define START_TRACING_UNTIL_END_OF_SCOPE lease_tracer leased_tracer;

// TODO: logically belongs in main.cc with the rest of the test harness
long Num_failures = 0;
#define CHECK_EQ(X, Y) if ((X) != (Y)) { \
    ++Num_failures; \
    cerr << "\nF " << __FUNCTION__ << ": " << #X << " == " << #Y << '\n'; \
    cerr << "  got " << (X) << '\n';  /* BEWARE: multiple eval */ \
    return; \
  } \
  else { cerr << "."; fflush(stderr); }

#define check_trace_contents(layer, expected) \
  CHECK_EQ(global_trace_stream->contents(layer), expected);
