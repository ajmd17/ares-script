#ifndef CONFIG_H
#define CONFIG_H

namespace avm {
namespace config {

static const bool optimize_constant_folding = true;
static const bool optimize_remove_unused = true;
static const bool optimize_remove_dead_code = true;

static const bool write_labels_to_beginning = false; // breaks with the new way of storing functions

} // namespace config
} // namespace avm

#endif