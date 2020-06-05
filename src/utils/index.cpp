#include <albedo/utils/identifier.h>

namespace albedo
{

std::atomic<Identifier::Size> GlobalIdentifier::globalId = 0;

} // namespace albedo
