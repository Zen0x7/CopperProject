#include <copper/state.h>

 copper::state::state() : id_(boost::uuids::random_generator()()) {}
