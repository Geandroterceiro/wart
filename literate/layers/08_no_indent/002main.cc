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

int main(int argc, unused char* argv[]) {
  if (argc > 1) {
    run_tests();
    return 0;
  }

  //// Interactive loop: parse commands from user, evaluate them, print the results
  setup();
  load_files(".wart");
  cout << "ready! type in an expression, then hit enter twice. ctrl-d exits.\n";
  while (!cin.eof()) {
    cell* form = read(cin);
    cell* result = eval(form);
    cout << "=> " << result << '\n';
    rmref(result);
    rmref(form);
  }
}

//// read: tokenize, segment, parse, build cells, transform $vars
cell* read(istream& in) {
  return mkref(transform_dollar_vars(next_cell(in)));
}

extern cell* nil;

cell* run(istream& in) {
  cell* result = nil;
  do {
      cell* form = read(in);
      update(result, eval(form));
      rmref(form);
  } while (!eof(in));
  return result;
}

bool eof(istream& in) {
  in.peek();
  return in.eof();
}



//// test harness

void run_tests() {
  time_t t; time(&t);
  cerr << "C tests: " << ctime(&t);
  for (unsigned long i=0; i < sizeof(Tests)/sizeof(Tests[0]); ++i) {
    START_TRACING_UNTIL_END_OF_SCOPE;
    setup();
    (*Tests[i])();
    verify();
  }

  setup();
  load_files(".wart");   // after GC tests
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
  Hide_warnings = false;
  Passed = true;
}



//// helpers for tests

void read_all(string s) {
  stringstream in(s);
  do {
      rmref(read(in));
  } while (!eof(in));
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
