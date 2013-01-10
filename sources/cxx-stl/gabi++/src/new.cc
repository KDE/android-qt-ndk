// Copyright (C) 2011 The Android Open Source Project
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
//

#include <stdlib.h>
#include <new>

namespace std {

  const nothrow_t nothrow = {};

  bad_alloc::bad_alloc() throw() {
  }

  bad_alloc::~bad_alloc() throw() {
  }

  const char* bad_alloc::what() const throw() {
    return "std::bad_alloc";
  }

} // namespace std

__attribute__ ((weak))
void* operator new(std::size_t size) throw(std::bad_alloc) {
  void* space = ::operator new(size, std::nothrow_t());
  if (space) {
    return space;
  } else {
    throw std::bad_alloc();
  }
}

__attribute__ ((weak))
void* operator new(std::size_t size, const std::nothrow_t& no) throw() {
  return malloc(size);
}

__attribute__ ((weak))
void* operator new[](std::size_t size) throw(std::bad_alloc) {
  return ::operator new(size);
}

__attribute__ ((weak))
void* operator new[](std::size_t size, const std::nothrow_t& no) throw() {
  return ::operator new[](size, no);
}
