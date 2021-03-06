//// run unit tests or interactive interpreter

// The design of lisp seems mindful of the following:
//  keep programs as short as possible
//  as programs grow, allow them to be decomposed into functions
//  functions are defined with a recipe for work, and invoked to perform that work
//  functions are parameterized with vars (parameters) and invoked with data (arguments)
//  new functions can be defined at any time
//  functions can invoke other functions before they've been defined
//  functions are data; they can be passed to other functions
//  when an invocation contains multiple functions, make the one being invoked easy to identify
//    almost always the first symbol in the invocation
//  there have to be *some* bedrock primitives, but any primitive can be user-defined in principle
//
// These weren't the reasons lisp was created; they're the reasons I attribute
// to its power.

bool Interactive = false;
bool Warn_on_unknown_var = true;

int main(int argc, const char* argv[]) {
  if (argc > 1) {
    bool early_exit = process_args(argc, argv);
    if (early_exit) return 0;
  }

  //// Interactive loop: parse commands from user, evaluate them, print the results
  setup();
  cerr << "starting up...       (takes ~15 seconds)\n";
  load_files(".wart");
  cerr << "ready! type in an expression, then hit enter twice. ctrl-d exits.\n";
  Interactive = true;  // stop run on two enters
  while (!cin.eof()) {
    cell* curr = run(cin);
    cout << "=> " << curr << '\n';
    rmref(curr);
  }
}

//// read: tokenize, parenthesize, parse, transform infix, build cells, transform $vars
// simply returns nil on eof
cell* read(indent_sensitive_stream& in) {
  return mkref(transform_dollar_vars(next_cell(in)));
}

// parenthesize requires input stream to know when it's at start of line
struct indent_sensitive_stream {
  istream& fd;
  bool at_start_of_line;
  explicit indent_sensitive_stream(istream& in) :fd(in), at_start_of_line(true) { fd >> std::noskipws; }
  // leaky version just for convenient tests
  explicit indent_sensitive_stream(string s) :fd(*new stringstream(s)), at_start_of_line(true) { fd >> std::noskipws; }
  bool eof() { return fd.eof(); }
};

extern cell* nil;

cell* run(istream& i) {
  indent_sensitive_stream in(i);
  cell* result = nil;
  while (true) {
    cell* form = read(in);
    update(result, eval(form));
    rmref(form);
    if (in.eof()) break;
    if (Interactive && at_end_of_line(in))
      // 'in' has no state left; destroy and print
      break;
  }
  return result;
}

bool at_end_of_line(indent_sensitive_stream& in) {
  if (in.at_start_of_line) return true;
  skip_whitespace(in.fd);
  if (in.fd.peek() == '#')
    skip_comment(in.fd);
  return in.fd.peek() == '\n';
}

string Last_file = "";

bool process_args(int argc, const char* argv[]) {
  bool test = false, tangle = false;
  for (int i = 1; i < argc; ++i) {
    string arg(argv[i]);
    if (arg == "test")
      test = true;
    else if (arg == "tangle")
      tangle = true;
    else if (arg == "--until")
      Last_file = argv[++i];
  }
  if (test) run_tests();
  else if (tangle) tangle_files_in_cwd();
  else return false;  // keep going
  return true;  // early exit
}



//// test harness

void run_tests() {
  cerr << time_string() << " C tests\n";
  for (unsigned long i=0; i < sizeof(Tests)/sizeof(Tests[0]); ++i) {
    START_TRACING_UNTIL_END_OF_SCOPE;
    setup();
    (*Tests[i])();
    verify();
  }

  setup();
  cerr << "\n" << time_string() << " loading wart files       (takes ~15 seconds)\n";
  load_files(".wart");  // after GC tests
  cerr << time_string() << " wart tests\n";
  load_files(".test");

  cerr << '\n';
  if (Num_failures > 0)
    cerr << Num_failures << " failure"
         << (Num_failures > 1 ? "s" : "")
         << '\n';
}

void verify() {
  Hide_warnings = false;
  teardown_streams();
  teardown_compiledfns();
  teardown_cells();
  if (!Passed)
    ;
  else if (num_unfreed() > 0)
    dump_unfreed();
  else
    cerr << ".";
}

void setup() {
  setup_cells();
  setup_common_syms();
  setup_scopes();
  setup_compiledfns();
  setup_streams();
  Interactive = false;
  Hide_warnings = false;
  Passed = true;
}



//// helpers for tests

void read_all(string s) {
  stringstream ss(s);
  indent_sensitive_stream in(ss);
  do {
      rmref(read(in));
  } while (!in.eof());
  // return nothing; we'll just verify the trace
}

void run(string s) {
  stringstream in(s);
  rmref(run(in));
  // return nothing; we'll just verify the trace
}

cell* read(string s) {
  return read(*new stringstream(s));
}

string time_string() {
  time_t t;
  time(&t);
  char buffer[10];
  if (!strftime(buffer, 10, "%H:%M:%S", localtime(&t)))
    return "";
  return buffer;
}
