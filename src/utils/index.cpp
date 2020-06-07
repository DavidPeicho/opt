#include <albedo/utils/index.h>

namespace albedo
{

std::atomic<GlobalIdentifier::Type> GlobalIdentifier::globalId = 0;

} // namespace albedo
