#pragma once

#include "dto/BallisticTableDTO.hpp"

#include <memory>

// Returns nullptr if source is empty/unreadable/malformed.
std::unique_ptr<BallisticTableDTO> load_ballistic_table(const char* source);
