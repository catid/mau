/** \file
    \brief Mau Implementation: Pre-compiled header
    \copyright Copyright (c) 2017 Christopher A. Taylor.  All rights reserved.
*/

#pragma once

// When doing large refactors it can be helpful to turn off PCH so that
// single modules can be test-compiled in isolation.
#define MAU_ENABLE_PCH

#ifdef MAU_ENABLE_PCH
#include "MauProxy.h"
#include "Logger.h"
#include "PacketAllocator.h"
#endif // MAU_ENABLE_PCH
