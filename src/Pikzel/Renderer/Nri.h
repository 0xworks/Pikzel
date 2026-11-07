#pragma once

#include "Pikzel/Renderer/Format.h"

#include <NRI.h>

#include <Extensions/NRIDeviceCreation.h>
#include <Extensions/NRIHelper.h>
#include <Extensions/NRISwapChain.h>

#include <string_view>

// Renderer-internal NRI declarations. Only the renderer's .cpp files include this header.
//
// NVIDIA NRI is the graphics API abstraction the renderer is built on. It is an
// implementation detail: the renderer's public headers only forward-declare the NRI types
// they refer to, so code using the renderer (games, examples, the editor) neither sees
// nor needs NRI. Pikzel links NRI privately for the same reason.
//
// NRI exposes its functionality as tables of function pointers ("interfaces"), fetched
// from a device at run time. The core interface covers resources, pipelines and command
// recording; extensions such as swap chains live in separate interfaces. The renderer
// gathers the interfaces it uses into one struct, so that calls read as NRI.CreateBuffer(...)
// regardless of which interface a function belongs to. (This is the pattern used by
// NVIDIA's own NRI samples.)
namespace Pikzel {

struct NriInterface
: nri::CoreInterface,
  nri::HelperInterface,
  nri::SwapChainInterface {};

std::string_view ToString(nri::Result result);

// Throws std::runtime_error if result is not SUCCESS. Most NRI failures (out of memory,
// invalid arguments, a lost device) leave nothing sensible to do but stop, so they are
// reported as exceptions with the name of the operation that failed.
void CheckNri(nri::Result result, std::string_view operation);

nri::Format ToNri(Format format);

// Returns Format::Unknown for NRI formats the engine does not define.
Format FromNri(nri::Format format);

}
